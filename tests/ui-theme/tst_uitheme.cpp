#include <QtTest>
#include <QGuiApplication>
#include <QNetworkProxy>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlComponent>
#include <QQuickItem>
#include <QQuickWindow>
#include <QQuickStyle>
#include <QSettings>
#include <QTemporaryDir>

#include "settings/streamingpreferences.h"
#include "backend/systemproperties.h"
#include "backend/autoupdatechecker.h"
#include "gui/computermodel.h"
#include "gui/appmodel.h"
#include "gui/sdlgamepadkeynavigation.h"
#include "path.h"
#include "streaming/session.h"
#include "settings/localshortcuts.h"
#include <QScopeGuard>
#include <QTranslator>
#include <QDesktopServices>

QAtomicInt g_AsyncLoggingEnabled;

class TransferHandler : public QObject
{
    Q_OBJECT
public:
    QUrl received;
public slots:
    void open(const QUrl& url) { received = url; }
};

class UIThemeTest : public QObject
{
    Q_OBJECT
    QQmlApplicationEngine m_Engine;

    static QQuickItem* findVisual(QQuickItem* root, const QString& name)
    {
        if (root->objectName() == name) return root;
        for (auto child : root->childItems()) {
            if (auto found = findVisual(child, name)) return found;
        }
        return nullptr;
    }

private slots:
    void init()
    {
        QSettings settings;
        settings.clear();
        settings.setValue("mdns", false);
        settings.setValue("detectnetblocking", false);
        StreamingPreferences::get()->reload();
    }

    void shortcutEditor()
    {
        auto& engine = m_Engine;
        engine.rootContext()->setContextProperty("initialView", "SettingsView.qml");
        engine.rootContext()->setContextProperty("isGuiLaunch", false);
        engine.load(QUrl("qrc:/gui/main.qml"));
        QVERIFY(!engine.rootObjects().isEmpty());
        auto window = qobject_cast<QQuickWindow*>(engine.rootObjects().first());
        QVERIFY(window);
        auto windowCleanup = qScopeGuard([&] { delete window; });
        QVERIFY(QTest::qWaitForWindowExposed(window));
        auto stats = findVisual(window->contentItem(), "shortcutKey_stats");
        auto reserved = findVisual(window->contentItem(), "shortcutKey_ungrab");
        auto save = window->findChild<QObject*>("saveShortcuts");
        auto reset = window->findChild<QObject*>("resetShortcuts");
        auto error = window->findChild<QObject*>("shortcutError");
        QVERIFY(stats && reserved && save && reset && error);
        QVERIFY(reserved->property("readOnly").toBool());
        QVERIFY(stats->setProperty("text", "t"));
        QVERIFY(QMetaObject::invokeMethod(save, "clicked"));
        QCOMPARE(StreamingPreferences::get()->localShortcutKeys[3], QString("t"));
        QVERIFY(!error->property("visible").toBool());
        stats = findVisual(window->contentItem(), "shortcutKey_stats");
        QVERIFY(stats);
        QVERIFY(stats->setProperty("text", "z"));
        QVERIFY(QMetaObject::invokeMethod(save, "clicked"));
        QVERIFY(error->property("visible").toBool());
        QCOMPARE(QSettings().value("localshortcuts/stats").toString(), QString("t"));
        QVERIFY(QMetaObject::invokeMethod(reset, "clicked"));
        QCOMPARE(StreamingPreferences::get()->localShortcutKeys, LocalShortcuts::defaults());
        QVERIFY(!QSettings().contains("localshortcuts/stats"));
        window->showNormal();
        QTest::qWait(250);
        window->setWidth(720);
        window->setHeight(600);
        QTRY_COMPARE(window->width(), 720);
        QTest::qWait(100);
        QTRY_COMPARE(window->width(), 720);
        auto versionLabel = window->findChild<QObject*>("versionLabel");
        QVERIFY(versionLabel);
        QVERIFY(!versionLabel->property("visible").toBool());
        stats = findVisual(window->contentItem(), "shortcutKey_stats");
        QVERIFY(stats);
        QVERIFY(QMetaObject::invokeMethod(stats, "forceActiveFocus"));
        QTRY_VERIFY(stats->property("activeFocus").toBool());
        const auto preview = qEnvironmentVariable("PERIGEE_THEME_PREVIEW_DIR");
        if (!preview.isEmpty()) {
            QDir().mkpath(preview);
            QTest::qWait(200);
            QVERIFY(window->grabWindow().save(preview + "/shortcuts.png"));
        }
    }

    void streamControlDispatch_data()
    {
        QTest::addColumn<QString>("locale");
        QTest::newRow("english") << QString();
        for (const auto& file : QDir(":/languages").entryList({"qml_*.qm"})) {
            QTest::newRow(qPrintable(file)) << file;
        }
    }

    void streamControlDispatch()
    {
        QFETCH(QString, locale);
        QTranslator translator;
        if (!locale.isEmpty()) {
            QVERIFY(translator.load(":/languages/" + locale));
            QCoreApplication::installTranslator(&translator);
        }
        auto translationCleanup = qScopeGuard([&] { QCoreApplication::removeTranslator(&translator); });
        auto navigation = m_Engine.singletonInstance<SdlGamepadKeyNavigation*>(qmlTypeId("SdlGamepadKeyNavigation", 1, 0, "SdlGamepadKeyNavigation"));
        QVERIFY(navigation);
        navigation->disable();
        auto navigationCleanup = qScopeGuard([&] { navigation->enable(); });
        QVERIFY(SDL_InitSubSystem(SDL_INIT_VIDEO) == 0);
        auto videoCleanup = qScopeGuard([] { SDL_QuitSubSystem(SDL_INIT_VIDEO); });
        SDL_Window* window = SDL_CreateWindow("Synthetic controls", 0, 0, 960, 540, SDL_WINDOW_HIDDEN);
        QVERIFY(window);
        auto windowCleanup = qScopeGuard([&] { SDL_DestroyWindow(window); });
        NvComputer computer{};
        NvApp app{};
        auto prefs = StreamingPreferences::get();
        QSettings().setValue("localshortcuts/stats", "t");
        QSettings().setValue("localshortcuts/mouse", "");
        prefs->reload();
        Session session(&computer, app, prefs);
        Session::s_ActiveSession = &session;
        auto sessionCleanup = qScopeGuard([] { Session::s_ActiveSession = nullptr; });
        SdlInputHandler input(*prefs, 1920, 1080);
        input.setWindow(window);
        QVERIFY(input.m_SpecialKeyCombos[SdlInputHandler::KeyComboToggleMouseMode].enabled);
        QCOMPARE(input.m_SpecialKeyCombos[SdlInputHandler::KeyComboToggleMouseMode].keyCode, SDLK_UNKNOWN);
        class Renderer : public Overlay::IOverlayRenderer {
        public:
            void notifyOverlayUpdated(Overlay::OverlayType) override {}
        } renderer;
        auto& overlay = session.getOverlayManager();
        overlay.setOverlayRenderer(&renderer);
        auto rendererCleanup = qScopeGuard([&] { overlay.setOverlayRenderer(nullptr); });
        auto press = [&](SDL_Keycode key, SDL_Scancode scan, Uint16 modifiers = 0, Uint8 state = SDL_PRESSED) {
            SDL_KeyboardEvent event{};
            event.keysym.sym = key;
            event.keysym.scancode = scan;
            event.keysym.mod = modifiers;
            event.state = state;
            input.handleKeyEvent(&event);
        };
        const Uint16 chord = KMOD_CTRL | KMOD_ALT | KMOD_SHIFT;
        press(SDLK_t, SDL_SCANCODE_T, chord);
        QVERIFY(overlay.isOverlayEnabled(Overlay::OverlayDebug));
        QVERIFY(input.m_KeysDown.isEmpty());
        press(SDLK_t, SDL_SCANCODE_T, 0, SDL_RELEASED);
        QVERIFY(input.m_KeysDown.isEmpty());
        press(SDLK_m, SDL_SCANCODE_M, chord);
        QVERIFY(!input.m_KeysDown.isEmpty());
        input.raiseAllKeys();
        press(SDLK_s, SDL_SCANCODE_S, chord);
        QVERIFY(!input.m_KeysDown.isEmpty());
        input.raiseAllKeys();
        press(SDLK_t, SDL_SCANCODE_T, chord | KMOD_MODE);
        QVERIFY(!input.m_KeysDown.isEmpty());
        input.raiseAllKeys();
        QVERIFY(!input.isSystemKeyCaptureActive());
        press(SDLK_LGUI, SDL_SCANCODE_LGUI, KMOD_GUI);
        QVERIFY(input.m_KeysDown.isEmpty());
        press(SDLK_LGUI, SDL_SCANCODE_LGUI, 0, SDL_RELEASED);
        QVERIFY(input.m_KeysDown.isEmpty());
        press(SDLK_QUOTE, SDL_SCANCODE_APOSTROPHE);
        QVERIFY(!input.m_KeysDown.isEmpty());
        press(SDLK_QUOTE, SDL_SCANCODE_APOSTROPHE, 0, SDL_RELEASED);
        QVERIFY(input.m_KeysDown.isEmpty());
        press(SDLK_a, SDL_SCANCODE_A);
        QVERIFY(!input.m_KeysDown.isEmpty());
        press(SDLK_o, SDL_SCANCODE_O, chord);
        QVERIFY(overlay.isCommandPanelVisible());
        QVERIFY(input.m_KeysDown.isEmpty());
        press(SDLK_o, SDL_SCANCODE_O, 0, SDL_RELEASED);
        press(SDLK_5, SDL_SCANCODE_5);
        QVERIFY(overlay.isCommandPanelVisible());
        QVERIFY(input.m_KeysDown.isEmpty());
        const auto preview = qEnvironmentVariable("PERIGEE_THEME_PREVIEW_DIR");
        SDL_Surface* surface = overlay.getUpdatedOverlaySurface(Overlay::OverlayStatusUpdate);
        QVERIFY(surface);
        if (!preview.isEmpty()) {
            QDir().mkpath(preview);
            QImage image(static_cast<const uchar*>(surface->pixels), surface->w, surface->h, surface->pitch, QImage::Format_ARGB32);
            QVERIFY(image.save(preview + "/stream-controls.png"));
        }
        SDL_FreeSurface(surface);
        press(SDLK_7, SDL_SCANCODE_7);
        QVERIFY(!overlay.isCommandPanelVisible());
        QVERIFY(!overlay.isOverlayEnabled(Overlay::OverlayDebug));
        press(SDLK_7, SDL_SCANCODE_7, 0, SDL_RELEASED);
        QVERIFY(input.m_KeysDown.isEmpty());
        press(SDLK_o, SDL_SCANCODE_O, chord);
        QVERIFY(overlay.isCommandPanelVisible());
        press(SDLK_ESCAPE, SDL_SCANCODE_ESCAPE);
        QVERIFY(!overlay.isCommandPanelVisible());
        press(SDLK_o, SDL_SCANCODE_O, chord);
        QVERIFY(overlay.isCommandPanelVisible());
        input.notifyFocusLost();
        QVERIFY(!overlay.isCommandPanelVisible());
        press(SDLK_o, SDL_SCANCODE_O, chord);
        QVERIFY(overlay.isCommandPanelVisible());
        press(SDLK_z, SDL_SCANCODE_Z, chord);
        QVERIFY(!overlay.isCommandPanelVisible());
        QVERIFY(!input.isCaptureActive());
        press(SDLK_z, SDL_SCANCODE_Z, chord);
        QVERIFY(!input.isCaptureActive());
        press(SDLK_o, SDL_SCANCODE_O, chord);
        QVERIFY(overlay.isCommandPanelVisible());
        press(SDLK_q, SDL_SCANCODE_Q, chord);
        QVERIFY(!overlay.isCommandPanelVisible());
        QVERIFY(SDL_HasEvent(SDL_QUIT));
        SDL_FlushEvent(SDL_QUIT);
    }

    void sessionProfileEditorAndSessionOwnership()
    {
        auto prefs = StreamingPreferences::get();
        const auto original = prefs->sessionPreferencesSnapshot();
        auto& engine = m_Engine;
        engine.rootContext()->setContextProperty("initialView", "SettingsView.qml");
        engine.rootContext()->setContextProperty("isGuiLaunch", false);
        engine.load(QUrl("qrc:/gui/main.qml"));
        QVERIFY(!engine.rootObjects().isEmpty());
        auto window = qobject_cast<QQuickWindow*>(engine.rootObjects().first());
        QVERIFY(window);
        auto windowCleanup = qScopeGuard([&] { delete window; });
        QVERIFY(QTest::qWaitForWindowExposed(window));
        auto selector = window->findChild<QObject*>("sessionProfileSelector");
        auto edit = window->findChild<QObject*>("editSessionProfile");
        auto dialog = window->findChild<QObject*>("sessionProfileDialog");
        QVERIFY(selector && edit && dialog);
        QVERIFY(selector->setProperty("currentIndex", 1));
        QVERIFY(QMetaObject::invokeMethod(selector, "activated", Q_ARG(int, 1)));
        QCOMPARE(prefs->sessionProfile(), QString("programming"));
        QVERIFY(QMetaObject::invokeMethod(edit, "clicked"));
        QTRY_VERIFY(dialog->property("opened").toBool());
        auto background = dialog->property("background").value<QObject*>();
        QVERIFY(background);
        QCOMPARE(background->property("color").value<QColor>(), QColor("#303030"));
        QVERIFY(prefs->setProperty("uiTheme", StreamingPreferences::UI_THEME_LIGHT));
        QTRY_COMPARE(background->property("color").value<QColor>(), QColor("#fafafa"));
        QVERIFY(prefs->setProperty("uiTheme", StreamingPreferences::UI_THEME_DARK));
        auto width = findVisual(window->contentItem(), "profileWidth");
        QVERIFY(width);
        auto save = findVisual(window->contentItem(), "saveSessionProfile");
        QVERIFY(save);
        QVERIFY(width->setProperty("text", "0"));
        QVERIFY(QMetaObject::invokeMethod(save, "clicked"));
        QVERIFY(dialog->property("opened").toBool());
        QVERIFY(!QSettings().contains("sessionprofiles/programming"));
        QVERIFY(width->setProperty("text", "2560"));
        QVERIFY(QMetaObject::invokeMethod(save, "clicked"));
        QTRY_VERIFY(!dialog->property("opened").toBool());
        QCOMPARE(prefs->sessionProfileValues("programming").value("width").toInt(), 2560);
        QCOMPARE(prefs->sessionPreferencesSnapshot(), original);
        NvComputer computer{};
        NvApp app{};
        Session first(&computer, app);
        QCOMPARE(first.m_Preferences->width, 2560);
        QVERIFY(first.m_Preferences != prefs);
        auto updated = prefs->sessionProfileValues("programming");
        updated["width"] = 3840;
        QVERIFY(prefs->saveSessionProfile("programming", updated));
        Session second(&computer, app);
        QCOMPARE(second.m_Preferences->width, 3840);
        QCOMPARE(first.m_Preferences->width, 2560);
        Session cli(&computer, app, prefs);
        QCOMPARE(cli.m_Preferences->sessionPreferencesSnapshot(), original);
        QCOMPARE(prefs->sessionPreferencesSnapshot(), original);
        QVERIFY(QMetaObject::invokeMethod(edit, "clicked"));
        QTRY_VERIFY(dialog->property("opened").toBool());
        auto pacing = findVisual(window->contentItem(), "profilePacing");
        auto scroll = findVisual(window->contentItem(), "profileScroll");
        QVERIFY(pacing && scroll);
        pacing->forceActiveFocus(Qt::TabFocusReason);
        QTRY_VERIFY(pacing->mapToItem(scroll, QPointF()).y() >= 0);
        QTRY_VERIFY(pacing->mapToItem(scroll, QPointF()).y() + pacing->height() <= scroll->height());
        width->forceActiveFocus(Qt::TabFocusReason);
        QTest::qWait(150);
        const auto preview = qEnvironmentVariable("PERIGEE_THEME_PREVIEW_DIR");
        if (!preview.isEmpty()) {
            QDir().mkpath(preview);
            QVERIFY(window->grabWindow().save(preview + "/profiles.png"));
        }
        QVERIFY(QMetaObject::invokeMethod(dialog, "reject"));
    }

    void transferDialogPresentation()
    {
        auto& engine = m_Engine;
        engine.rootContext()->setContextProperty("initialView", "PcView.qml");
        engine.rootContext()->setContextProperty("isGuiLaunch", false);
        engine.load(QUrl("qrc:/gui/main.qml"));
        QVERIFY(!engine.rootObjects().isEmpty());
        auto window = qobject_cast<QQuickWindow*>(engine.rootObjects().first());
        QVERIFY(window);
        auto windowCleanup = qScopeGuard([&] { delete window; });
        QVERIFY(QTest::qWaitForWindowExposed(window));
        auto dialog = window->findChild<QObject*>("transferDialog");
        QVERIFY(dialog);
        QVERIFY(QMetaObject::invokeMethod(dialog, "open"));
        QTRY_VERIFY(dialog->property("opened").toBool());
        auto background = dialog->property("background").value<QObject*>();
        QVERIFY(background);
        QCOMPARE(background->property("color").value<QColor>().alpha(), 255);
        auto field = findVisual(window->contentItem(), "transferText");
        QVERIFY(field);
        QTRY_VERIFY(field->hasActiveFocus());
        QVERIFY(field->setProperty("text", "sftp://user:secret@host/"));
        auto save = findVisual(window->contentItem(), "saveTransferUrl");
        QVERIFY(save);
        QVERIFY(QMetaObject::invokeMethod(save, "clicked"));
        auto error = findVisual(window->contentItem(), "transferError");
        QVERIFY(error && error->isVisible());
        QVERIFY(dialog->property("opened").toBool());
        const auto preview = qEnvironmentVariable("PERIGEE_THEME_PREVIEW_DIR");
        if (!preview.isEmpty()) {
            QDir().mkpath(preview);
            QVERIFY(window->grabWindow().save(preview + "/sftp.png"));
        }
        QVERIFY(QMetaObject::invokeMethod(dialog, "reject"));
        auto failure = window->findChild<QObject*>("transferOpenError");
        QVERIFY(failure);
        QVERIFY(QMetaObject::invokeMethod(failure, "open"));
        QTRY_VERIFY(failure->property("opened").toBool());
        auto failureBackground = failure->property("background").value<QObject*>();
        QVERIFY(failureBackground);
        QCOMPARE(failureBackground->property("color").value<QColor>().alpha(), 255);
        QVERIFY(QMetaObject::invokeMethod(failure, "reject"));
    }

    void appIdentityParser()
    {
        const QString uuid = "12345678-1234-1234-abcd-1234567890ab";
        const QString prefix = "<root status_code=\"200\"><App><AppTitle>Fixture</AppTitle><ID>17</ID>";
        const QString suffix = "</App></root>";
        const auto standard = NvHTTP::parseAppList(prefix + suffix);
        QCOMPARE(standard.size(), 1);
        QCOMPARE(standard[0].id, 17);
        QVERIFY(standard[0].uuid.isEmpty());
        const auto extended = NvHTTP::parseAppList(prefix + "<UUID>" + uuid + "</UUID>" + suffix);
        QCOMPARE(extended[0].uuid, uuid);
        for (const auto& field : {"<UUID>invalid</UUID>", "<UUID>00000000-0000-0000-0000-000000000000</UUID>"})
            QVERIFY(NvHTTP::parseAppList(prefix + field + suffix)[0].uuid.isEmpty());
        QVERIFY(NvHTTP::parseAppList(prefix + "<UUID>" + uuid + "</UUID><UUID>" + uuid + "</UUID>" + suffix)[0].uuid.isEmpty());
    }

    void transferModelAndHostPersistence()
    {
        const QString uuid = "12345678-1234-1234-abcd-1234567890ab";
        {
            QSettings settings;
            settings.beginWriteArray("hosts", 1);
            settings.setArrayIndex(0);
            settings.setValue("uuid", uuid);
            settings.setValue("name", "Synthetic host");
            settings.endArray();
        }
        {
            ComputerManager manager(StreamingPreferences::get());
            ComputerModel model;
            model.initialize(&manager);
            QCOMPARE(model.rowCount(QModelIndex()), 1);
            QVERIFY(!model.setTransferUrl(uuid, "sftp://user:secret@host/"));
            QVERIFY(!model.setTransferUrl("missing", "sftp://host/path"));
            QVERIFY(model.setTransferUrl(uuid, "sftp://host:2222/path"));
            TransferHandler handler;
            QDesktopServices::setUrlHandler("sftp", &handler, "open");
            auto cleanup = qScopeGuard([] { QDesktopServices::unsetUrlHandler("sftp"); });
            QVERIFY(model.openTransferUrl(uuid));
            QCOMPARE(handler.received, QUrl("sftp://host:2222/path"));
            QVERIFY(!model.openTransferUrl("missing"));
            QVERIFY(!model.setUseAppUuid(uuid, true));
            auto host = manager.getComputers().first();
            host->state = NvComputer::CS_ONLINE;
            host->apolloCapabilities.authenticated = true;
            host->apolloCapabilities.advertised = true;
            host->apolloCapabilities.permissionsState = ApolloCapabilities::State::Available;
            host->apolloCapabilities.permissionMask = ApolloCapabilities::Launch;
            NvApp app;
            app.id = 17;
            app.name = "Synthetic";
            app.uuid = uuid;
            host->appList = {app};
            QVERIFY(model.setUseAppUuid(uuid, true));
            QVERIFY(host->useAppUuid);
            host->state = NvComputer::CS_OFFLINE;
            QVERIFY(model.setUseAppUuid(uuid, false));
            host->state = NvComputer::CS_ONLINE;
            QVERIFY(model.setUseAppUuid(uuid, true));
        }
        {
            ComputerManager restarted(StreamingPreferences::get());
            QCOMPARE(restarted.getComputers().size(), 1);
            QCOMPARE(restarted.getComputers().first()->transferUrl, QString("sftp://host:2222/path"));
            QVERIFY(restarted.getComputers().first()->useAppUuid);
            ComputerModel model;
            model.initialize(&restarted);
            QVERIFY(model.setUseAppUuid(uuid, false));
            QVERIFY(model.setTransferUrl(uuid, ""));
            QVERIFY(!model.openTransferUrl(uuid));
        }
        {
            ComputerManager cleared(StreamingPreferences::get());
            QCOMPARE(cleared.getComputers().size(), 1);
            QVERIFY(cleared.getComputers().first()->transferUrl.isEmpty());
            QVERIFY(!cleared.getComputers().first()->useAppUuid);
        }
    }

    void savedValues_data()
    {
        QTest::addColumn<QVariant>("stored");
        QTest::addColumn<int>("expected");
        QTest::newRow("legacy-default") << QVariant() << 0;
        QTest::newRow("dark") << QVariant(0) << 0;
        QTest::newRow("light") << QVariant(1) << 1;
        QTest::newRow("negative") << QVariant(-1) << 0;
        QTest::newRow("future-value") << QVariant(2) << 0;
        QTest::newRow("malformed") << QVariant("invalid") << 0;
    }

    void savedValues()
    {
        QFETCH(QVariant, stored);
        QFETCH(int, expected);
        if (stored.isValid()) QSettings().setValue("uitheme", stored);
        StreamingPreferences::get()->reload();
        QCOMPARE(int(StreamingPreferences::get()->uiTheme), expected);
    }

    void shortcutPreferencePersistence()
    {
        auto prefs = StreamingPreferences::get();
        QCOMPARE(prefs->localShortcutKeys[3], QString("s"));
        QSettings().setValue("localshortcuts/stats", "t");
        prefs->reload();
        QCOMPARE(prefs->localShortcutKeys[3], QString("t"));
        prefs->save();
        prefs->reload();
        QCOMPARE(prefs->localShortcutKeys[3], QString("t"));
        QCOMPARE(QSettings().value("localshortcuts/stats").toString(), QString("t"));
        QSettings().setValue("localshortcuts/stats", "z");
        prefs->reload();
        QCOMPARE(prefs->localShortcutKeys[3], QString("s"));
        QCOMPARE(prefs->localShortcutKeys[1], QString("z"));
    }

    void persistenceAndNotification()
    {
        auto prefs = StreamingPreferences::get();
        const auto bitrate = prefs->bitrateKbps;
        QSignalSpy changed(prefs, &StreamingPreferences::uiThemeChanged);
        QVERIFY(prefs->setProperty("uiTheme", StreamingPreferences::UI_THEME_LIGHT));
        QCOMPARE(changed.count(), 1);
        prefs->save();
        QCOMPARE(QSettings().value("uitheme").toInt(), 1);
        prefs->reload();
        QCOMPARE(prefs->uiTheme, StreamingPreferences::UI_THEME_LIGHT);
        QCOMPARE(prefs->bitrateKbps, bitrate);
        QVERIFY(prefs->setProperty("uiTheme", StreamingPreferences::UI_THEME_DARK));
        prefs->save();
        prefs->reload();
        QCOMPARE(prefs->uiTheme, StreamingPreferences::UI_THEME_DARK);
        QCOMPARE(QSettings().value("uitheme").toInt(), 0);
    }

    void applicationThemeAndPopup()
    {
        auto& engine = m_Engine;
        engine.rootContext()->setContextProperty("initialView", "SettingsView.qml");
        engine.rootContext()->setContextProperty("isGuiLaunch", false);
        engine.load(QUrl("qrc:/gui/main.qml"));
        QVERIFY(!engine.rootObjects().isEmpty());
        auto window = qobject_cast<QQuickWindow*>(engine.rootObjects().first());
        QVERIFY(window);
        auto windowCleanup = qScopeGuard([&] { delete window; });
        QVERIFY(QTest::qWaitForWindowExposed(window));
        QQuickItem* selector = nullptr;
        for (auto item : window->findChildren<QQuickItem*>()) {
            if (item->property("currentText").toString() == "Dark" && item->property("count").toInt() == 2) {
                selector = item;
                break;
            }
        }
        QVERIFY(selector);
        auto prefs = StreamingPreferences::get();
        QVERIFY(selector->setProperty("currentIndex", 1));
        QVERIFY(QMetaObject::invokeMethod(selector, "activated", Q_ARG(int, 1)));
        QCOMPARE(prefs->uiTheme, StreamingPreferences::UI_THEME_LIGHT);
        QCOMPARE(QSettings().value("uitheme").toInt(), 1);

        int toolbarTitles = 0;
        for (auto item : window->findChildren<QQuickItem*>()) {
            if (item->property("text").toString() == "Settings" && item->property("color").isValid()) {
                QCOMPARE(item->property("color").value<QColor>(), QColor("white"));
                toolbarTitles++;
            }
        }
        QVERIFY(toolbarTitles > 0);

        QQmlComponent icons(&engine);
        icons.setData("import QtQuick 2.9\nimport QtQuick.Controls.Material 2.2\nThemedImage { width: 48; height: 48 }", QUrl("qrc:/gui/theme-fixture.qml"));
        QScopedPointer<QObject> imageObject(icons.create());
        auto image = qobject_cast<QQuickItem*>(imageObject.data());
        QVERIFY2(image, qPrintable(icons.errorString()));
        image->setParentItem(window->contentItem());
        const QStringList names = {"desktop_windows-48px", "baseline-lock-24px", "baseline-help_outline-24px",
            "baseline-error_outline-24px", "baseline-check_circle_outline-24px", "baseline-warning-24px",
            "warning_FILL1_wght300_GRAD200_opsz24"};
        for (const auto& name : names) {
            QVERIFY(image->setProperty("iconSource", QUrl("qrc:/res/" + name + ".svg")));
            QTRY_COMPARE(image->property("status").toInt(), 1);
            QCOMPARE(image->property("source").toUrl(), QUrl("qrc:/res/" + name + "-light.svg"));
        }

        QObject* popup = selector->property("popup").value<QObject*>();
        QVERIFY(popup);
        QVERIFY(QMetaObject::invokeMethod(popup, "open"));
        QTRY_VERIFY(popup->property("opened").toBool());
        auto background = popup->property("background").value<QQuickItem*>();
        QVERIFY(background);
        QCOMPARE(background->property("color").value<QColor>(), QColor("#ffffff"));
        QVERIFY(prefs->setProperty("uiTheme", StreamingPreferences::UI_THEME_DARK));
        QTRY_COMPARE(background->property("color").value<QColor>(), QColor("#424242"));
        QVERIFY(!image->property("source").toString().contains("-light.svg"));
        QVERIFY(QMetaObject::invokeMethod(popup, "close"));
        const QString preview = qEnvironmentVariable("PERIGEE_THEME_PREVIEW_DIR");
        if (!preview.isEmpty()) {
            QDir().mkpath(preview);
            QTest::qWait(150);
            QVERIFY(window->grabWindow().save(preview + "/dark.png"));
            QVERIFY(prefs->setProperty("uiTheme", StreamingPreferences::UI_THEME_LIGHT));
            QTest::qWait(150);
            QVERIFY(window->grabWindow().save(preview + "/light.png"));
        }
    }
};

int main(int argc, char** argv)
{
    QTemporaryDir profile;
    if (!profile.isValid()) return 2;
    for (const auto& key : {"XDG_CONFIG_HOME", "XDG_CACHE_HOME", "XDG_DATA_HOME", "TMPDIR"}) {
        auto folder = profile.filePath(key);
        QDir().mkpath(folder);
        qputenv(key, QFile::encodeName(folder));
    }
    qputenv("QT_QUICK_CONTROLS_MATERIAL_PRIMARY", "#3F51B5");
    qputenv("QT_QUICK_CONTROLS_MATERIAL_ACCENT", "Purple");
    qputenv("QT_QUICK_CONTROLS_MATERIAL_VARIANT", "Dense");
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName("PerigeeThemeFixture");
    QCoreApplication::setApplicationName("SyntheticTests");
    Path::initialize(false);
    QQuickStyle::setStyle("Material");
    QNetworkProxy::setApplicationProxy(QNetworkProxy(QNetworkProxy::HttpProxy, "127.0.0.1", 9));
    qmlRegisterType<ComputerModel>("ComputerModel", 1, 0, "ComputerModel");
    qmlRegisterType<AppModel>("AppModel", 1, 0, "AppModel");
    qmlRegisterSingletonType<StreamingPreferences>("StreamingPreferences", 1, 0, "StreamingPreferences",
        [](QQmlEngine* engine, QJSEngine*) -> QObject* { return StreamingPreferences::get(engine); });
    qmlRegisterSingletonType<SystemProperties>("SystemProperties", 1, 0, "SystemProperties",
        [](QQmlEngine*, QJSEngine*) -> QObject* { return new SystemProperties(); });
    qmlRegisterSingletonType<ComputerManager>("ComputerManager", 1, 0, "ComputerManager",
        [](QQmlEngine* engine, QJSEngine*) -> QObject* { return new ComputerManager(StreamingPreferences::get(engine)); });
    qmlRegisterSingletonType<AutoUpdateChecker>("AutoUpdateChecker", 1, 0, "AutoUpdateChecker",
        [](QQmlEngine*, QJSEngine*) -> QObject* { return new AutoUpdateChecker(); });
    qmlRegisterSingletonType<SdlGamepadKeyNavigation>("SdlGamepadKeyNavigation", 1, 0, "SdlGamepadKeyNavigation",
        [](QQmlEngine* engine, QJSEngine*) -> QObject* { return new SdlGamepadKeyNavigation(StreamingPreferences::get(engine)); });
    UIThemeTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "tst_uitheme.moc"
