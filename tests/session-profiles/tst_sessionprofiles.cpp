#include <QtTest>
#include <QSettings>
#include <QTemporaryDir>
#include <QProcess>
#include <QMetaProperty>
#include <QScopeGuard>
#include "settings/streamingpreferences.h"
#include "utils.h"

bool WMUtils::isRunningWayland() { return false; }
bool WMUtils::isGpuSlow() { return false; }

class SessionProfilesTest : public QObject
{
    Q_OBJECT
    QTemporaryDir m_Profile;
private slots:
    void initTestCase()
    {
        QVERIFY(m_Profile.isValid());
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_Profile.path());
        QCoreApplication::setOrganizationName("PerigeeSyntheticProfiles");
        QCoreApplication::setApplicationName("SnapshotChecks");
    }
    void init()
    {
        QFile::remove(QSettings().fileName());
        StreamingPreferences::get()->reload();
    }
    void snapshotIsolation()
    {
        auto prefs = StreamingPreferences::get();
        const auto snapshot = prefs->sessionPreferencesSnapshot();
        QCOMPARE(snapshot.size(), 12);
        auto draft = snapshot;
        draft["width"] = 3840;
        draft["fps"] = 60;
        QCOMPARE(prefs->sessionPreferencesSnapshot(), snapshot);
        const int width = prefs->width;
        prefs->width++;
        QCOMPARE(snapshot.value("width").toInt(), width);
        prefs->width = width;
        QCOMPARE(prefs->sessionPreferencesSnapshot(), snapshot);
        for (const auto& key : {"certificate", "key", "uniqueid", "hosts", "localshortcuts", "clipboardenabled"})
            QVERIFY(!snapshot.contains(key));
        QVERIFY(!QFile::exists(QSettings().fileName()));
    }
    void presetsAndValidation()
    {
        auto prefs = StreamingPreferences::get();
        prefs->fps = 120;
        prefs->enableHdr = true;
        prefs->enableYUV444 = true;
        const auto original = prefs->sessionPreferencesSnapshot();
        const auto programming = prefs->sessionProfileDefaults("programming");
        QCOMPARE(programming.value("fps").toInt(), 60);
        QVERIFY(!programming.value("hdr").toBool());
        QVERIFY(!programming.value("yuv444").toBool());
        QCOMPARE(programming.value("videocfg").toInt(), int(StreamingPreferences::VCC_AUTO));
        QCOMPARE(programming.value("bitrate").toInt(), StreamingPreferences::getDefaultBitrate(prefs->width, prefs->height, 60, false));
        QCOMPARE(prefs->sessionProfileDefaults("gaming"), original);
        QVERIFY(prefs->sessionProfileDefaults("../escape").isEmpty());
        QVERIFY(StreamingPreferences::validSessionProfile(programming));
        for (const auto& change : QList<QVariantMap>{{{"width", 0}}, {{"fps", 481}}, {{"bitrate", -1}},
                    {{"width", true}}, {{"fps", 60.5}}, {{"hdr", "true"}}, {{"videocfg", 3}}, {{"videodec", 9}}, {{"unknown", 1}}}) {
            auto invalid = programming;
            for (auto field = change.cbegin(); field != change.cend(); ++field) invalid[field.key()] = field.value();
            QVERIFY(!StreamingPreferences::validSessionProfile(invalid));
            QVERIFY(!prefs->saveSessionProfile("programming", invalid));
        }
        QCOMPARE(prefs->sessionPreferencesSnapshot(), original);
    }

    void savedProfileAndSessionIsolation()
    {
        auto prefs = StreamingPreferences::get();
        const auto original = prefs->sessionPreferencesSnapshot();
        prefs->packetSize = 1300;
        auto draft = prefs->sessionProfileDefaults("programming");
        draft["width"] = 2560;
        QVERIFY(prefs->saveSessionProfile("programming", draft));
        QVERIFY(prefs->selectSessionProfile("programming"));
        auto first = prefs->createSessionPreferences();
        QCOMPARE(first->width, 2560);
        QCOMPARE(first->packetSize, 1300);
        QCOMPARE(prefs->sessionPreferencesSnapshot(), original);
        draft["width"] = 3840;
        QVERIFY(prefs->saveSessionProfile("programming", draft));
        auto second = prefs->createSessionPreferences();
        QCOMPARE(first->width, 2560);
        QCOMPARE(second->width, 3840);
        first->width = 1280;
        QCOMPARE(second->width, 3840);
        QCOMPARE(prefs->width, original.value("width").toInt());
        auto cli = prefs->createSessionPreferences(false);
        QCOMPARE(cli->sessionPreferencesSnapshot(), original);
        QProcess probe;
        probe.start(QCoreApplication::applicationFilePath(), {"--profile-probe", m_Profile.path()});
        QVERIFY(probe.waitForFinished(5000));
        QCOMPARE(probe.exitStatus(), QProcess::NormalExit);
        QCOMPARE(probe.exitCode(), 0);
        prefs->reload();
        QCOMPARE(prefs->sessionProfile(), QString("programming"));
        QCOMPARE(prefs->sessionProfileValues("programming").value("width").toInt(), 3840);
        QVERIFY(!prefs->selectSessionProfile("unknown"));
        QCOMPARE(prefs->sessionProfile(), QString("programming"));
        QVERIFY(prefs->selectSessionProfile(""));
        QCOMPARE(prefs->createSessionPreferences()->sessionPreferencesSnapshot(), original);
    }

    void detachedBasePreferences()
    {
        auto prefs = StreamingPreferences::get();
        prefs->playAudioOnHost = true;
        prefs->multiController = false;
        prefs->connectionWarnings = false;
        prefs->absoluteMouseMode = true;
        prefs->captureSysKeysMode = StreamingPreferences::CSK_ALWAYS;
        prefs->rendererSelection = StreamingPreferences::RS_VULKAN;
        prefs->packetSize = 1300;
        prefs->localShortcutKeys[3] = "t";
        const auto detached = prefs->createSessionPreferences(false);
        for (int index = QObject::staticMetaObject.propertyCount(); index < prefs->metaObject()->propertyCount(); index++) {
            const QMetaProperty property = prefs->metaObject()->property(index);
            if (property.isWritable()) QCOMPARE(property.read(detached.get()), property.read(prefs));
        }
        QCOMPARE(detached->packetSize, 1300);
        QCOMPARE(detached->localShortcutKeys, prefs->localShortcutKeys);
        prefs->playAudioOnHost = false;
        prefs->localShortcutKeys[3] = "s";
        QVERIFY(detached->playAudioOnHost);
        QCOMPARE(detached->localShortcutKeys[3], QString("t"));
    }

    void failedStorageDoesNotApplyDraft()
    {
        const QString blocked = m_Profile.filePath("not-a-directory");
        QFile file(blocked);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.close();
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, blocked);
        auto restore = qScopeGuard([&] {
            QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_Profile.path());
        });
        auto prefs = StreamingPreferences::get();
        const auto original = prefs->sessionPreferencesSnapshot();
        QVERIFY(!prefs->selectSessionProfile("programming"));
        QVERIFY(prefs->sessionProfile().isEmpty());
        auto draft = prefs->sessionProfileDefaults("programming");
        draft["width"] = 3840;
        QVERIFY(!prefs->saveSessionProfile("programming", draft));
        QVERIFY(!QSettings().value("sessionprofiles/programming").isValid());
        QCOMPARE(prefs->createSessionPreferences()->sessionPreferencesSnapshot(), original);
    }

    void corruptProfileFallback()
    {
        auto prefs = StreamingPreferences::get();
        auto stored = prefs->sessionProfileDefaults("programming");
        stored["certificate"] = "not-an-identity";
        QSettings().setValue("sessionprofiles/programming", stored);
        QVERIFY(prefs->selectSessionProfile("programming"));
        QCOMPARE(prefs->sessionProfileValues("programming"), prefs->sessionProfileDefaults("programming"));
        QVERIFY(!prefs->createSessionPreferences()->sessionPreferencesSnapshot().contains("certificate"));
    }

};
int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    if (app.arguments().size() == 3 && app.arguments()[1] == "--profile-probe") {
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, app.arguments()[2]);
        app.setOrganizationName("PerigeeSyntheticProfiles");
        app.setApplicationName("SnapshotChecks");
        auto prefs = StreamingPreferences::get();
        return prefs->sessionProfile() == "programming" && prefs->createSessionPreferences()->width == 3840 ? 0 : 1;
    }
    SessionProfilesTest test;
    return QTest::qExec(&test, argc, argv);
}
#include "tst_sessionprofiles.moc"
