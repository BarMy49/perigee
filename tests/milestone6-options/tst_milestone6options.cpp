#include <QtTest>
#include <limits>
#include <QTemporaryDir>
#include "backend/appuuidlaunch.h"
#include "backend/hostadministration.h"
#include "backend/nvapp.h"
#include "backend/servercommandrequest.h"

class Milestone6OptionsTest : public QObject
{
    Q_OBJECT
private slots:
    void sftpUrls()
    {
        for (const auto& text : {"sftp://host/path", "sftp://[::1]:2222/home", "sftp://host/", "sftp://host/My%20Documents"})
            QVERIFY(!HostAdministration::validatedSftpUrl(text).isEmpty());
        for (const auto& text : {"https://host/", "sftp:///path", "sftp://user@host/", "sftp://user:secret@host/",
                 "sftp://host:0/", "sftp://host:65536/", "sftp://host/?query", "sftp://host/#fragment",
                 "sftp://host/%0a", "sftp://host/a b", "sftp://host/%00", ""})
            QVERIFY2(HostAdministration::validatedSftpUrl(text).isEmpty(), text);
    }
    void missingTransferHandler()
    {
        bool called = false;
        QVERIFY(!HostAdministration::openSftpUrl("sftp://host/path", [&](const QUrl&) {
            called = true;
            return false;
        }));
        QVERIFY(called);
        called = false;
        QVERIFY(!HostAdministration::openSftpUrl("sftp://user:secret@host/path", [&](const QUrl&) {
            called = true;
            return true;
        }));
        QVERIFY(!called);
        QVERIFY(HostAdministration::openSftpUrl("sftp://host/path", [](const QUrl& url) {
            return url == QUrl("sftp://host/path");
        }));
    }

    void appUuidPolicy()
    {
        const QString uuid = "12345678-1234-1234-abcd-1234567890AB";
        QCOMPARE(AppUuidLaunch::normalized(uuid), uuid.toLower());
        QCOMPARE(AppUuidLaunch::query(17, uuid, false), QString("appid=17"));
        QCOMPARE(AppUuidLaunch::query(17, uuid, true), QString("appid=0&appuuid=") + uuid.toLower());
        QCOMPARE(AppUuidLaunch::query(17, "invalid", true), QString("appid=17"));
        for (const auto& value : {"", "{12345678-1234-1234-abcd-1234567890ab}", "00000000-0000-0000-0000-000000000000", "not-a-uuid", "&appuuid=bad"})
            QVERIFY(AppUuidLaunch::normalized(value).isEmpty());
        ApolloCapabilities capabilities;
        capabilities.authenticated = true;
        capabilities.advertised = true;
        capabilities.permissionsState = ApolloCapabilities::State::Available;
        capabilities.permissionMask = ApolloCapabilities::Launch;
        QVERIFY(AppUuidLaunch::eligible(false, true, capabilities, true, true, uuid));
        QVERIFY(!AppUuidLaunch::eligible(true, true, capabilities, true, true, uuid));
        QVERIFY(!AppUuidLaunch::eligible(false, false, capabilities, true, true, uuid));
        QVERIFY(!AppUuidLaunch::eligible(false, true, capabilities, false, true, uuid));
        QVERIFY(!AppUuidLaunch::eligible(false, true, capabilities, true, false, uuid));
        capabilities.authenticated = false;
        QVERIFY(!AppUuidLaunch::eligible(false, true, capabilities, true, true, uuid));
        capabilities.authenticated = true;
        capabilities.permissionMask = ApolloCapabilities::View;
        QVERIFY(!AppUuidLaunch::eligible(false, true, capabilities, true, true, uuid));
    }
    void ambiguousAppIdentities()
    {
        NvApp app;
        app.id = 17;
        app.uuid = "12345678-1234-1234-abcd-1234567890ab";
        QVector<NvApp> apps{app};
        QVERIFY(AppUuidLaunch::matchesAppIdentity(apps, 17, app.uuid));
        QVERIFY(!AppUuidLaunch::matchesAppIdentity(apps, 18, app.uuid));
        auto duplicate = app;
        duplicate.id = 18;
        apps.append(duplicate);
        QVERIFY(!AppUuidLaunch::matchesAppIdentity(apps, 17, app.uuid));
        duplicate.id = 17;
        duplicate.uuid = "87654321-1234-1234-abcd-1234567890ab";
        apps = {app, duplicate};
        QVERIFY(!AppUuidLaunch::matchesAppIdentity(apps, 17, app.uuid));
        QVERIFY(!AppUuidLaunch::matchesAppIdentity(apps, 0, app.uuid));
    }

    void pinnedAppList()
    {
        const QUrl origin("https://host:47984/");
        const QUrl reply("https://host:47984/applist?uniqueid=synthetic");
        QVERIFY(AppUuidLaunch::acceptsAppListReply(origin, reply, "pin", "pin"));
        QVERIFY(!AppUuidLaunch::acceptsAppListReply(origin, reply, "pin", "other"));
        QVERIFY(!AppUuidLaunch::acceptsAppListReply(origin, reply, "", ""));
        for (const auto& value : {"http://host:47984/applist", "https://other:47984/applist", "https://host:47985/applist",
                 "https://host:47984/serverinfo", "https://user@host:47984/applist", "https://host:47984/applist#fragment"})
            QVERIFY(!AppUuidLaunch::acceptsAppListReply(origin, QUrl(value), "pin", "pin"));
    }
    void serverCommandBoundary()
    {
        ApolloCapabilities capabilities;
        capabilities.advertised = true;
        capabilities.authenticated = true;
        capabilities.permissionsState = ApolloCapabilities::State::Available;
        capabilities.permissionMask = ApolloCapabilities::ServerCommand;
        capabilities.commandsState = ApolloCapabilities::State::Available;
        capabilities.commandNames = {"Second command", "First command"};
        const auto shown = capabilities.commandNames;
        auto first = ServerCommandRequest::prepare(capabilities, shown, 7, 7, true, 0);
        QVERIFY(first);
        QCOMPARE((*first)[0], quint8(0));
        auto request = ServerCommandRequest::prepare(capabilities, shown, 7, 7, true, 1);
        QVERIFY(request);
        const std::array<quint8, 4> expected{1, 0, 0, 0};
        QVERIFY(*request == expected);
        for (int index : {-1, 2, 255, 256, std::numeric_limits<int>::max()})
            QVERIFY(!ServerCommandRequest::prepare(capabilities, shown, 7, 7, true, index));
        QVERIFY(!ServerCommandRequest::prepare(capabilities, shown, 6, 7, true, 0));
        QVERIFY(!ServerCommandRequest::prepare(capabilities, shown, 0, 0, true, 0));
        QVERIFY(!ServerCommandRequest::prepare(capabilities, shown, 7, 7, false, 0));
        capabilities.commandNames.swapItemsAt(0, 1);
        QVERIFY(!ServerCommandRequest::prepare(capabilities, shown, 7, 7, true, 0));
        capabilities.commandNames = shown;
        capabilities.permissionMask = 0;
        QVERIFY(!ServerCommandRequest::prepare(capabilities, shown, 7, 7, true, 0));
        capabilities.permissionMask = ApolloCapabilities::ServerCommand;
        capabilities.authenticated = false;
        QVERIFY(!ServerCommandRequest::prepare(capabilities, shown, 7, 7, true, 0));
        capabilities.authenticated = true;
        capabilities.commandNames.clear();
        for (int index = 0; index < 256; index++) capabilities.commandNames.append(QString::number(index));
        request = ServerCommandRequest::prepare(capabilities, capabilities.commandNames, 7, 7, true, 255);
        QVERIFY(request);
        QCOMPARE((*request)[0], quint8(255));
        capabilities.commandNames.append("too many");
        QVERIFY(!ServerCommandRequest::prepare(capabilities, capabilities.commandNames, 7, 7, true, 0));
    }

    void appUuidPersistence()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        QSettings settings(directory.filePath("apps.ini"), QSettings::IniFormat);
        NvApp app;
        app.id = 17;
        app.name = "Synthetic";
        app.favorite = true;
        app.uuid = "12345678-1234-1234-abcd-1234567890ab";
        app.serialize(settings);
        settings.sync();
        NvApp restored(settings);
        QVERIFY(restored == app);
        restored.uuid.clear();
        QVERIFY(restored != app);
        settings.setValue("uuid", "invalid");
        QVERIFY(NvApp(settings).uuid.isEmpty());
    }
};
QTEST_GUILESS_MAIN(Milestone6OptionsTest)
#include "tst_milestone6options.moc"
