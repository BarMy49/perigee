#include <QtTest>

#include "backend/apollocapabilities.h"
#include "backend/hostadministration.h"

using State = ApolloCapabilities::State;

class ApolloCapabilitiesTest : public QObject
{
    Q_OBJECT

    static QString response(const QString& fields, const QString& paired = "1")
    {
        return "<root status_code=\"200\"><hostname>Fixture host</hostname><appversion>7.1.431.0</appversion>"
               "<GfeVersion>3.23.0.74</GfeVersion><PairStatus>" + paired + "</PairStatus>" + fields + "</root>";
    }

private slots:
    void standardHost()
    {
        for (bool pinned : {false, true}) {
            const auto value = ApolloCapabilities::parse(response("<FutureField>ignored</FutureField>"), pinned);
            QVERIFY(!value.advertised);
            QCOMPARE(value.permissionsState, State::Absent);
            QCOMPARE(value.commandsState, State::Absent);
            QCOMPARE(value.displayCapableState, State::Absent);
            QCOMPARE(value.displayDriverState, State::Absent);
            QCOMPARE(value.permission(ApolloCapabilities::Keyboard), State::Absent);
            QVERIFY(value.commandNames.isEmpty());
        }
        const auto versionOnly = ApolloCapabilities::parse(response("<ApolloVersion>invented</ApolloVersion>"), true);
        QVERIFY(!versionOnly.advertised);
    }

    void permissionStates_data()
    {
        QTest::addColumn<QString>("value");
        QTest::addColumn<bool>("pinned");
        QTest::addColumn<QString>("paired");
        QTest::addColumn<int>("expected");
        QTest::newRow("http-zero-is-not-denial") << QString("0") << false << QString("1") << int(State::Unauthenticated);
        QTest::newRow("http-full-is-not-granted") << QString("4294967295") << false << QString("1") << int(State::Unauthenticated);
        QTest::newRow("unpaired") << QString("4294967295") << true << QString("0") << int(State::Unauthenticated);
        QTest::newRow("unknown-pair-state") << QString("4294967295") << true << QString("unknown") << int(State::Unauthenticated);
        QTest::newRow("explicit-denial") << QString("0") << true << QString("1") << int(State::Denied);
        QTest::newRow("full-mask") << QString("4294967295") << true << QString("1") << int(State::Available);
        for (const QString& invalid : {QString("-1"), QString("+1"), QString("0x1000"), QString("1.0"), QString("4294967296"), QString(""), QString("no")}) {
            QTest::newRow(qPrintable("invalid-" + invalid)) << invalid << true << QString("1") << int(State::Invalid);
        }
    }

    void permissionStates()
    {
        QFETCH(QString, value);
        QFETCH(bool, pinned);
        QFETCH(QString, paired);
        QFETCH(int, expected);
        const auto parsed = ApolloCapabilities::parse(response("<Permission>" + value + "</Permission>", paired), pinned);
        QVERIFY(parsed.advertised);
        QCOMPARE(int(parsed.permissionsState), expected);
        if (parsed.permissionsState != State::Available) {
            QCOMPARE(parsed.permissionMask, quint32(0));
        }
        QCOMPARE(parsed.permission(0), State::Invalid);
    }

    void individualBitsAndUnknownBits()
    {
        const auto parsed = ApolloCapabilities::parse(response("<Permission>2147618816</Permission>"), true);
        QCOMPARE(parsed.permissionMask, quint32(0x80021000));
        QCOMPARE(parsed.permission(ApolloCapabilities::Keyboard), State::Available);
        QCOMPARE(parsed.permission(ApolloCapabilities::ClipboardRead), State::Available);
        QCOMPARE(parsed.permission(ApolloCapabilities::ClipboardSet), State::Denied);
        QCOMPARE(parsed.permission(ApolloCapabilities::Keyboard | ApolloCapabilities::ClipboardRead), State::Available);
        QCOMPARE(parsed.permission(ApolloCapabilities::Keyboard | ApolloCapabilities::Mouse), State::Denied);
        QCOMPARE(parsed.toVariantMap().value("permissionMask").toUInt(), quint32(0x80021000));
    }

    void commandsAreOrderedAndGated()
    {
        const QString commands = "<ServerCommand>Second</ServerCommand><ServerCommand>First</ServerCommand><ServerCommand>Second</ServerCommand>";
        const auto parsed = ApolloCapabilities::parse(response("<Permission>1048576</Permission>" + commands), true);
        QCOMPARE(parsed.commandsState, State::Available);
        QCOMPARE(parsed.commandNames, QStringList({"Second", "First", "Second"}));
        const auto denied = ApolloCapabilities::parse(response("<Permission>4096</Permission>" + commands), true);
        QCOMPARE(denied.commandsState, State::Denied);
        QVERIFY(denied.commandNames.isEmpty());
        const auto untrusted = ApolloCapabilities::parse(response("<Permission>1048576</Permission>" + commands), false);
        QCOMPARE(untrusted.commandsState, State::Unauthenticated);
        QVERIFY(untrusted.commandNames.isEmpty());
        const auto missingMask = ApolloCapabilities::parse(response(commands), true);
        QVERIFY(missingMask.commandNames.isEmpty());
        QCOMPARE(missingMask.commandsState, State::Unauthenticated);
        const auto absentList = ApolloCapabilities::parse(response("<Permission>1048576</Permission>"), true);
        QCOMPARE(absentList.commandsState, State::Absent);
    }

    void commandLimits()
    {
        QString commands;
        for (int index = 0; index < 256; index++) commands += "<ServerCommand>Command</ServerCommand>";
        auto value = ApolloCapabilities::parse(response("<Permission>1048576</Permission>" + commands), true);
        QCOMPARE(value.commandNames.size(), 256);
        value = ApolloCapabilities::parse(response("<Permission>1048576</Permission>" + commands + "<ServerCommand>Extra</ServerCommand>"), true);
        QCOMPARE(value.commandsState, State::Invalid);
        QVERIFY(value.commandNames.isEmpty());
        for (const QString& name : {QString(""), QString("   "), QString(513, 'x'), QString("line\nbreak")}) {
            value = ApolloCapabilities::parse(response("<Permission>1048576</Permission><ServerCommand>" + name + "</ServerCommand>"), true);
            QCOMPARE(value.commandsState, State::Invalid);
            QVERIFY(value.commandNames.isEmpty());
        }
    }

    void displayFlags()
    {
        const auto parsed = ApolloCapabilities::parse(response("<Permission>0</Permission><VirtualDisplayCapable>true</VirtualDisplayCapable><VirtualDisplayDriverReady>false</VirtualDisplayDriverReady>"), true);
        QCOMPARE(parsed.permissionsState, State::Denied);
        QCOMPARE(parsed.displayCapableState, State::Available);
        QVERIFY(parsed.displayCapable);
        QCOMPARE(parsed.displayDriverState, State::Available);
        QVERIFY(!parsed.displayDriverReady);
        QCOMPARE(parsed.permission(ApolloCapabilities::Launch), State::Denied);
        const auto untrusted = ApolloCapabilities::parse(response("<VirtualDisplayCapable>1</VirtualDisplayCapable><VirtualDisplayDriverReady>1</VirtualDisplayDriverReady>"), false);
        QCOMPARE(untrusted.displayCapableState, State::Unauthenticated);
        QVERIFY(!untrusted.displayCapable && !untrusted.displayDriverReady);
        const auto invalid = ApolloCapabilities::parse(response("<VirtualDisplayCapable>yes</VirtualDisplayCapable><VirtualDisplayDriverReady>2</VirtualDisplayDriverReady>"), true);
        QCOMPARE(invalid.displayCapableState, State::Invalid);
        QCOMPARE(invalid.displayDriverState, State::Invalid);
    }

    void duplicateFieldsAndNestedSpoofing()
    {
        const auto repeated = ApolloCapabilities::parse(response("<Permission>0</Permission><Permission>4294967295</Permission><VirtualDisplayCapable>true</VirtualDisplayCapable><VirtualDisplayCapable>false</VirtualDisplayCapable>"), true);
        QCOMPARE(repeated.permissionsState, State::Invalid);
        QCOMPARE(repeated.displayCapableState, State::Invalid);
        QCOMPARE(repeated.permissionMask, quint32(0));
        const auto nested = ApolloCapabilities::parse(response("<Unrelated><Permission>4294967295</Permission></Unrelated>"), true);
        QVERIFY(!nested.advertised);
        const auto namespaced = ApolloCapabilities::parse(response("<x:Permission xmlns:x=\"urn:unrelated\">4294967295</x:Permission>"), true);
        QVERIFY(!namespaced.advertised);
    }

    void rejectsMalformedXml_data()
    {
        QTest::addColumn<QString>("xml");
        QTest::newRow("truncated") << response("<Permission>4294967295");
        QTest::newRow("wrong-status") << QString("<root status_code=\"401\"><Permission>4294967295</Permission></root>");
        QTest::newRow("missing-status") << QString("<root><Permission>4294967295</Permission></root>");
        QTest::newRow("wrong-root") << QString("<other status_code=\"200\"><Permission>4294967295</Permission></other>");
        QTest::newRow("duplicate-pair-state") << response("<PairStatus>1</PairStatus><Permission>4294967295</Permission>");
        QTest::newRow("nested-text") << response("<Permission><nested>4294967295</nested></Permission>");
        QTest::newRow("doctype") << QString("<!DOCTYPE root [<!ENTITY field '4294967295'>]>") + response("<Permission>&field;</Permission>");
        QTest::newRow("second-root") << response("<Permission>4294967295</Permission>") + response("");
        QTest::newRow("trailing-junk") << response("<Permission>4294967295</Permission>") + "junk";
        QTest::newRow("oversized") << QString(1024 * 1024 + 1, 'x');
    }

    void rejectsMalformedXml()
    {
        QFETCH(QString, xml);
        const auto parsed = ApolloCapabilities::parse(xml, true);
        QCOMPARE(parsed.permissionsState, State::Invalid);
        QCOMPARE(parsed.permissionMask, quint32(0));
        QVERIFY(!parsed.authenticated);
        QVERIFY(parsed.commandNames.isEmpty());
    }

    void expiryAndReplacement()
    {
        const auto parsed = ApolloCapabilities::parse(response("<Permission>4294967295</Permission><ServerCommand>Action</ServerCommand><VirtualDisplayCapable>true</VirtualDisplayCapable><VirtualDisplayDriverReady>true</VirtualDisplayDriverReady>"), true);
        auto expired = parsed.withoutAuthentication();
        QVERIFY(!expired.authenticated);
        QCOMPARE(expired.permissionMask, quint32(0));
        QVERIFY(expired.commandNames.isEmpty());
        QVERIFY(!expired.displayCapable && !expired.displayDriverReady);
        QCOMPARE(expired.permissionsState, State::Unauthenticated);
        QCOMPARE(expired.commandsState, State::Unauthenticated);
        QVERIFY(expired.withoutAuthentication() == expired);
        expired = ApolloCapabilities::parse(response(""), true);
        QCOMPARE(expired.permissionsState, State::Absent);
        QVERIFY(expired.commandNames.isEmpty());
    }

    void pinnedReplyPolicy_data()
    {
        QTest::addColumn<QUrl>("origin");
        QTest::addColumn<QUrl>("reply");
        QTest::addColumn<QByteArray>("pin");
        QTest::addColumn<QByteArray>("peer");
        QTest::addColumn<bool>("expected");
        const QUrl origin("https://host:47984");
        const QUrl reply("https://host:47984/serverinfo?uniqueid=synthetic");
        QTest::newRow("pinned-custom-port") << origin << reply << QByteArray("pin") << QByteArray("pin") << true;
        QTest::newRow("http") << QUrl("http://host:47984") << QUrl("http://host:47984/serverinfo") << QByteArray("pin") << QByteArray("pin") << false;
        QTest::newRow("mismatch") << origin << reply << QByteArray("pin") << QByteArray("other") << false;
        QTest::newRow("missing-pin") << origin << reply << QByteArray() << QByteArray() << false;
        QTest::newRow("redirect-host") << origin << QUrl("https://other:47984/serverinfo") << QByteArray("pin") << QByteArray("pin") << false;
        QTest::newRow("redirect-port") << origin << QUrl("https://host:47985/serverinfo") << QByteArray("pin") << QByteArray("pin") << false;
        QTest::newRow("redirect-path") << origin << QUrl("https://host:47984/other") << QByteArray("pin") << QByteArray("pin") << false;
        QTest::newRow("userinfo") << origin << QUrl("https://user@host:47984/serverinfo") << QByteArray("pin") << QByteArray("pin") << false;
        QTest::newRow("port-zero") << QUrl("https://host:0") << QUrl("https://host:0/serverinfo") << QByteArray("pin") << QByteArray("pin") << false;
    }

    void pinnedReplyPolicy()
    {
        QFETCH(QUrl, origin);
        QFETCH(QUrl, reply);
        QFETCH(QByteArray, pin);
        QFETCH(QByteArray, peer);
        QFETCH(bool, expected);
        QCOMPARE(ApolloCapabilities::acceptsPinnedReply(origin, reply, pin, peer), expected);
    }

    void administrationUrls_data()
    {
        QTest::addColumn<QString>("text");
        QTest::addColumn<bool>("valid");
        QTest::newRow("custom-port") << QString("https://host.example:48000/") << true;
        QTest::newRow("reverse-proxy") << QString("https://host.example/admin/") << true;
        QTest::newRow("ipv6") << QString("https://[::1]:48000/") << true;
        for (const QString& invalid : {QString("http://host/"), QString("file:///etc/passwd"), QString("javascript:alert(1)"),
                                     QString("https://user:password@host/"), QString("https://@host/"), QString("https://host/?token=secret"),
                                     QString("https://host/#secret"), QString("https://host:0/"), QString("https://host:99999/"),
                                     QString("https://host/%0A"), QString("host:48000"), QString(2049, 'x'), QString("")}) {
            QTest::newRow(qPrintable("invalid-" + QString::number(invalid.size()) + "-" + invalid.left(12))) << invalid << false;
        }
    }

    void administrationUrls()
    {
        QFETCH(QString, text);
        QFETCH(bool, valid);
        QCOMPARE(!HostAdministration::validatedUrl(text).isEmpty(), valid);
    }
};

QTEST_GUILESS_MAIN(ApolloCapabilitiesTest)
#include "tst_apollocapabilities.moc"
