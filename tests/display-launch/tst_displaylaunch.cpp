#include <QtTest>
#include "backend/displaylaunchoptions.h"

class DisplayLaunchTest : public QObject
{
    Q_OBJECT

private slots:
    void gating_data()
    {
        using State = DisplayLaunchOptions::State;
        QTest::addColumn<bool>("requested");
        QTest::addColumn<bool>("isGfe");
        QTest::addColumn<bool>("freshLaunch");
        QTest::addColumn<bool>("pinned");
        QTest::addColumn<QString>("fields");
        QTest::addColumn<QString>("paired");
        QTest::addColumn<int>("expected");
        const QString ready = "<Permission>67108864</Permission><VirtualDisplayCapable>true</VirtualDisplayCapable><VirtualDisplayDriverReady>true</VirtualDisplayDriverReady>";
        QTest::newRow("host-default-no-probes-or-extension") << false << false << true << false << QString() << QString("0") << int(State::HostDefault);
        QTest::newRow("host-default-even-on-ready-host") << false << false << true << true << ready << QString("1") << int(State::HostDefault);
        QTest::newRow("standard-sunshine") << true << false << true << true << QString() << QString("1") << int(State::Unsupported);
        QTest::newRow("gfe") << true << true << true << true << ready << QString("1") << int(State::Unsupported);
        QTest::newRow("untrusted-metadata") << true << false << true << false << ready << QString("1") << int(State::Unauthenticated);
        QTest::newRow("unpaired") << true << false << true << true << ready << QString("0") << int(State::Unauthenticated);
        QTest::newRow("launch-denied") << true << false << true << true << QString(ready).replace("67108864", "33554432") << QString("1") << int(State::PermissionDenied);
        QTest::newRow("invalid-permissions") << true << false << true << true << QString(ready).replace("67108864", "invalid") << QString("1") << int(State::PermissionDenied);
        QTest::newRow("missing-driver") << true << false << true << true << QString(ready).replace("<VirtualDisplayDriverReady>true</VirtualDisplayDriverReady>", "") << QString("1") << int(State::DriverUnavailable);
        QTest::newRow("driver-not-ready") << true << false << true << true << QString(ready).replace("DriverReady>true", "DriverReady>false") << QString("1") << int(State::DriverUnavailable);
        QTest::newRow("invalid-driver") << true << false << true << true << QString(ready).replace("DriverReady>true", "DriverReady>maybe") << QString("1") << int(State::DriverUnavailable);
        QTest::newRow("not-capable") << true << false << true << true << QString(ready).replace("Capable>true", "Capable>false") << QString("1") << int(State::Unsupported);
        QTest::newRow("resume-or-running-app") << true << false << false << true << ready << QString("1") << int(State::NeedsFreshLaunch);
        QTest::newRow("ready-new-launch") << true << false << true << true << ready << QString("1") << int(State::Ready);
    }

    void gating()
    {
        QFETCH(bool, requested);
        QFETCH(bool, isGfe);
        QFETCH(bool, freshLaunch);
        QFETCH(bool, pinned);
        QFETCH(QString, fields);
        QFETCH(QString, paired);
        QFETCH(int, expected);
        const auto capabilities = ApolloCapabilities::parse("<root status_code=\"200\"><PairStatus>" + paired + "</PairStatus>" + fields + "</root>", pinned);
        const auto state = DisplayLaunchOptions::evaluate(requested, isGfe, freshLaunch, capabilities);
        QCOMPARE(int(state), expected);
        QCOMPARE(DisplayLaunchOptions::query(state), state == DisplayLaunchOptions::State::Ready ? QString("&virtualDisplay=1") : QString());
        QVERIFY(!DisplayLaunchOptions::description(state).isEmpty());
    }

    void revokedSnapshotCannotAuthorizeLaunch()
    {
        auto capabilities = ApolloCapabilities::parse("<root status_code=\"200\"><PairStatus>1</PairStatus><Permission>67108864</Permission><VirtualDisplayCapable>1</VirtualDisplayCapable><VirtualDisplayDriverReady>1</VirtualDisplayDriverReady></root>", true);
        QCOMPARE(DisplayLaunchOptions::evaluate(true, false, true, capabilities), DisplayLaunchOptions::State::Ready);
        capabilities = capabilities.withoutAuthentication();
        const auto state = DisplayLaunchOptions::evaluate(true, false, true, capabilities);
        QCOMPARE(state, DisplayLaunchOptions::State::Unauthenticated);
        QVERIFY(DisplayLaunchOptions::query(state).isEmpty());
    }
};

QTEST_GUILESS_MAIN(DisplayLaunchTest)
#include "tst_displaylaunch.moc"
