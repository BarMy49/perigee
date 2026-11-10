#include <QtTest>
#include <QGuiApplication>
#include <QAbstractItemModelTester>
#include <QNetworkProxy>
#include <QPersistentModelIndex>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickView>
#include <QSettings>
#include <QTemporaryDir>
#include <memory>

#include "gui/appmodel.h"
#include "gui/sdlgamepadkeynavigation.h"
#include "path.h"

QAtomicInt g_AsyncLoggingEnabled;
ComputerManager* g_FixtureManager = nullptr;

class AppFavoritesTest : public QObject
{
    Q_OBJECT

    std::unique_ptr<ComputerManager> m_Manager;
    std::unique_ptr<AppModel> m_Model;
    NvComputer* m_Host = nullptr;

    int role(const QByteArray& name) const
    {
        return m_Model->roleNames().key(name, -1);
    }

    QList<int> ids() const
    {
        QList<int> result;
        for (int row = 0; row < m_Model->rowCount(QModelIndex()); row++) {
            result.append(m_Model->data(m_Model->index(row), role("appid")).toInt());
        }
        return result;
    }

    void openModel()
    {
        m_Manager = std::make_unique<ComputerManager>(nullptr);
        g_FixtureManager = m_Manager.get();
        for (NvComputer* host : m_Manager->getComputers()) {
            host->state = NvComputer::CS_ONLINE;
            host->pairState = NvComputer::PS_PAIRED;
            const QString directory = Path::getBoxArtCacheDir() + "/" + host->uuid;
            QDir().mkpath(directory);
            QImage image(200, 267, QImage::Format_RGB32);
            image.fill(Qt::gray);
            for (const NvApp& app : host->appList) {
                QVERIFY(image.save(directory + "/" + QString::number(app.id) + ".png"));
            }
        }
        m_Host = m_Manager->getComputers().first();
        m_Model = std::make_unique<AppModel>();
        m_Model->initialize(m_Manager.get(), 0, false);
        new QAbstractItemModelTester(m_Model.get(), QAbstractItemModelTester::FailureReportingMode::QtTest, m_Model.get());
    }

private slots:
    void init()
    {
        QSettings settings;
        settings.clear();
        settings.beginWriteArray("hosts", 2);
        for (int hostIndex = 0; hostIndex < 2; hostIndex++) {
            settings.setArrayIndex(hostIndex);
            settings.setValue("hostname", QString("Fixture %1").arg(hostIndex));
            settings.setValue("uuid", QString("favorites-fixture-%1").arg(hostIndex));
            settings.beginWriteArray("apps", 4);
            const QStringList names = {"Alpha", "Browser", "Code", "Hidden"};
            for (int appIndex = 0; appIndex < names.count(); appIndex++) {
                settings.setArrayIndex(appIndex);
                NvApp app;
                app.id = appIndex + 1;
                app.name = names.at(appIndex);
                app.hidden = appIndex == 3;
                app.directLaunch = appIndex == 1;
                app.serialize(settings);
            }
            settings.endArray();
        }
        settings.endArray();
        settings.sync();
        openModel();
    }

    void cleanup()
    {
        m_Model.reset();
        m_Manager.reset();
        g_FixtureManager = nullptr;
    }

    void legacyDefaults()
    {
        QTemporaryDir directory;
        QSettings settings(directory.filePath("legacy.ini"), QSettings::IniFormat);
        settings.setValue("id", 1);
        settings.setValue("name", "Legacy");
        QVERIFY(!NvApp(settings).favorite);
        QCOMPARE(ids(), QList<int>({1, 2, 3}));
        QCOMPARE(m_Model->getDirectLaunchAppIndex(), 1);
    }

    void reorderPreservesIdentity()
    {
        QPersistentModelIndex selected(m_Model->index(2));
        QSignalSpy moves(m_Model.get(), &QAbstractItemModel::rowsMoved);
        QSignalSpy resets(m_Model.get(), &QAbstractItemModel::modelReset);
        m_Model->setAppFavorite(3, true);
        QCOMPARE(ids(), QList<int>({3, 1, 2}));
        QCOMPARE(selected.row(), 0);
        QCOMPARE(selected.data(role("appid")).toInt(), 3);
        QCOMPARE(moves.count(), 1);
        QCOMPARE(resets.count(), 0);
        m_Model->setAppFavorite(1, true);
        QCOMPARE(ids(), QList<int>({1, 3, 2}));
        m_Model->setAppFavorite(3, false);
        QCOMPARE(ids(), QList<int>({1, 2, 3}));
        QVERIFY(!m_Host->appList.at(2).favorite);
        QVERIFY(m_Host->appList.at(1).directLaunch);
        QVERIFY(m_Host->appList.at(3).hidden);
    }

    void invalidAndUnchangedRequests()
    {
        QSignalSpy changed(m_Manager.get(), &ComputerManager::computerStateChanged);
        m_Model->setAppFavorite(-1, true);
        m_Model->setAppFavorite(9999, true);
        m_Model->setAppFavorite(1, false);
        QCOMPARE(changed.count(), 0);
        QCOMPARE(ids(), QList<int>({1, 2, 3}));
    }

    void refreshedHostPreservesFavorite()
    {
        m_Model->setAppFavorite(3, true);
        NvComputer refreshed = *m_Host;
        for (NvApp& app : refreshed.appList) {
            app.favorite = false;
            app.hidden = false;
            app.directLaunch = false;
            if (app.id == 3) app.name = "Zulu";
        }
        NvApp added;
        added.id = 5;
        added.name = "Aardvark";
        refreshed.appList.append(added);
        m_Host->update(refreshed);
        m_Manager->clientSideAttributeUpdated(m_Host);
        QCOMPARE(ids(), QList<int>({3, 5, 1, 2}));
        QVERIFY(m_Model->data(m_Model->index(0), role("favorite")).toBool());
        QCOMPARE(m_Model->data(m_Model->index(0), role("name")).toString(), QString("Zulu"));
        QVERIFY(m_Host->appList.at(2).directLaunch);
        QVERIFY(m_Host->appList.at(3).hidden);
        QPersistentModelIndex removed(m_Model->index(0));
        refreshed.appList.removeLast();
        for (int index = 0; index < refreshed.appList.count(); index++) {
            if (refreshed.appList.at(index).id == 3) refreshed.appList.removeAt(index);
        }
        m_Host->update(refreshed);
        m_Manager->clientSideAttributeUpdated(m_Host);
        QCOMPARE(ids(), QList<int>({1, 2}));
        QVERIFY(!removed.isValid());
    }

    void renameReordersExistingRows()
    {
        QPersistentModelIndex selected(m_Model->index(0));
        NvComputer refreshed = *m_Host;
        refreshed.appList[0].name = "Zulu";
        m_Host->update(refreshed);
        m_Manager->clientSideAttributeUpdated(m_Host);
        QCOMPARE(ids(), QList<int>({2, 3, 1}));
        QCOMPARE(selected.row(), 2);
        QCOMPARE(selected.data(role("appid")).toInt(), 1);
    }

    void hiddenFavoriteStaysHidden()
    {
        m_Model->setAppFavorite(4, true);
        QCOMPARE(ids(), QList<int>({1, 2, 3}));
        AppModel allApps;
        allApps.initialize(m_Manager.get(), 0, true);
        QCOMPARE(allApps.rowCount(QModelIndex()), 4);
        QCOMPARE(allApps.data(allApps.index(0), role("appid")).toInt(), 4);
        QVERIFY(allApps.data(allApps.index(0), role("favorite")).toBool());
    }

    void hostIsolationAndRestart()
    {
        m_Model->setAppFavorite(3, true);
        QVERIFY(!m_Manager->getComputers().at(1)->appList.at(2).favorite);
        m_Model.reset();
        m_Manager.reset();
        openModel();
        QCOMPARE(ids(), QList<int>({3, 1, 2}));
        QVERIFY(m_Host->appList.at(2).favorite);
        QVERIFY(!m_Manager->getComputers().at(1)->appList.at(2).favorite);
        m_Model->setAppFavorite(3, false);
        m_Model.reset();
        m_Manager.reset();
        openModel();
        QCOMPARE(ids(), QList<int>({1, 2, 3}));
        QVERIFY(!m_Host->appList.at(2).favorite);
    }

    void existingActionsAfterReorder()
    {
        m_Model->setAppFavorite(3, true);
        m_Host->currentGameId = 2;
        m_Manager->clientSideAttributeUpdated(m_Host);
        QCOMPARE(m_Model->getRunningAppId(), 2);
        QCOMPARE(m_Model->getRunningAppName(), QString("Browser"));
        m_Model->setAppDirectLaunch(0, true);
        QVERIFY(m_Host->appList.at(2).directLaunch);
        QVERIFY(!m_Host->appList.at(1).directLaunch);
        QCOMPARE(m_Model->getDirectLaunchAppIndex(), 0);
        m_Model->setAppHidden(0, true);
        QVERIFY(m_Host->appList.at(2).hidden);
        QCOMPARE(ids().first(), 3);
    }

    void pendingBoxArtFollowsAppId()
    {
        NvApp pending = m_Host->appList.at(2);
        m_Model->setAppFavorite(3, true);
        QSignalSpy changes(m_Model.get(), &QAbstractItemModel::dataChanged);
        QVERIFY(QMetaObject::invokeMethod(m_Model.get(), "handleBoxArtLoaded", Qt::DirectConnection,
            Q_ARG(NvComputer*, m_Host), Q_ARG(NvApp, pending), Q_ARG(QUrl, QUrl("file:///synthetic.png"))));
        QCOMPARE(changes.count(), 1);
        QCOMPARE(changes.first().at(0).value<QModelIndex>().row(), 0);
        QCOMPARE(changes.first().at(2).value<QList<int>>(), QList<int>({role("boxart")}));
    }

    void gridSelectionFollowsApp()
    {
        QQuickView view;
        view.rootContext()->setContextProperty("favoritesModel", m_Model.get());
        view.setSource(QUrl::fromLocalFile(QFINDTESTDATA("selection.qml")));
        QCOMPARE(view.status(), QQuickView::Ready);
        view.show();
        QVERIFY(QTest::qWaitForWindowExposed(&view));
        QQuickItem* grid = view.rootObject()->findChild<QQuickItem*>("selectionGrid");
        QVERIFY(grid);
        QVERIFY(grid->setProperty("currentIndex", 2));
        m_Model->setAppFavorite(3, true);
        QTRY_COMPARE(grid->property("currentIndex").toInt(), 0);
        QQuickItem* selected = qvariant_cast<QQuickItem*>(grid->property("currentItem"));
        QVERIFY(selected);
        QCOMPARE(selected->property("selectedAppId").toInt(), 3);
    }

    void appViewFavoriteAction()
    {
        QQuickView view;
        view.setSource(QUrl::fromLocalFile(QFINDTESTDATA("appview.qml")));
        QCOMPARE(view.status(), QQuickView::Ready);
        view.show();
        QVERIFY(QTest::qWaitForWindowExposed(&view));
        QQuickItem* grid = view.rootObject()->findChild<QQuickItem*>("appGrid");
        QVERIFY(grid);
        QVERIFY(grid->setProperty("currentIndex", 2));
        QQuickItem* selected = qvariant_cast<QQuickItem*>(grid->property("currentItem"));
        QVERIFY(selected);
        QTRY_VERIFY(selected->property("appContextMenu").value<QObject*>());
        QObject* menu = selected->property("appContextMenu").value<QObject*>();
        QVERIFY(QMetaObject::invokeMethod(selected, "contextMenuRequested"));
        QTRY_VERIFY(menu->property("opened").toBool());
        QObject* favoriteAction = nullptr;
        for (QObject* child : menu->findChildren<QObject*>()) {
            if (child->property("checkable").toBool() && child->property("text").toString() == "Favorite") {
                favoriteAction = child;
                break;
            }
        }
        QVERIFY(favoriteAction);
        QVERIFY(QMetaObject::invokeMethod(favoriteAction, "triggered"));
        QTRY_VERIFY(!menu->property("visible").toBool());
        QCOMPARE(ids(), QList<int>({3, 1, 2}));
        QTRY_COMPARE(grid->property("currentIndex").toInt(), 0);
        QCOMPARE(grid->property("currentItem").value<QQuickItem*>(), selected);
        QVERIFY(favoriteAction->property("checked").toBool());
        QVERIFY(QMetaObject::invokeMethod(favoriteAction, "triggered"));
        QCOMPARE(ids(), QList<int>({1, 2, 3}));
        QTRY_COMPARE(grid->property("currentIndex").toInt(), 2);
        QCOMPARE(grid->property("currentItem").value<QQuickItem*>(), selected);
    }
};

int main(int argc, char** argv)
{
    QTemporaryDir root;
    if (!root.isValid()) return 2;
    for (const auto& variable : {"XDG_CONFIG_HOME", "XDG_CACHE_HOME", "XDG_DATA_HOME", "TMPDIR"}) {
        const QByteArray directory = QFile::encodeName(root.filePath(variable));
        QDir().mkpath(QString::fromLocal8Bit(directory));
        qputenv(variable, directory);
    }
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName("PerigeeFavoritesFixture");
    QCoreApplication::setApplicationName("SyntheticTests");
    QSettings::setDefaultFormat(QSettings::IniFormat);
    Path::initialize(false);
    QNetworkProxy::setApplicationProxy(QNetworkProxy(QNetworkProxy::HttpProxy, "127.0.0.1", 9));
    qmlRegisterType<AppModel>("AppModel", 1, 0, "AppModel");
    qmlRegisterSingletonType<ComputerManager>("ComputerManager", 1, 0, "ComputerManager",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
            QQmlEngine::setObjectOwnership(g_FixtureManager, QQmlEngine::CppOwnership);
            return g_FixtureManager;
        });
    qmlRegisterSingletonType<SdlGamepadKeyNavigation>("SdlGamepadKeyNavigation", 1, 0, "SdlGamepadKeyNavigation",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
            return new SdlGamepadKeyNavigation(nullptr);
        });
    AppFavoritesTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "tst_appfavorites.moc"
