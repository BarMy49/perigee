#include <QtTest>
#include <QFile>
#include <QDir>
#include <QTranslator>
#include "settings/localshortcuts.h"
#include "path.h"
#include "streaming/video/overlaymanager.h"

QByteArray Path::readDataFile(QString fileName)
{
    QFile file(QFINDTESTDATA("../../app/" + fileName));
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

class TestRenderer : public Overlay::IOverlayRenderer
{
public:
    int updates = 0;
    void notifyOverlayUpdated(Overlay::OverlayType) override { updates++; }
};

class OverlayNotificationsTest : public QObject
{
    Q_OBJECT
private slots:
    void fullPanelLanguages()
    {
        const QDir directory(QFINDTESTDATA("../../app/languages"));
        const auto catalogs = directory.entryList({"qml_*.qm"});
        QCOMPARE(catalogs.size(), 30);
        for (const auto& catalog : catalogs) {
            QTranslator translator;
            QVERIFY(translator.load(directory.filePath(catalog)));
            QCoreApplication::installTranslator(&translator);
            const auto labels = LocalShortcuts::labels();
            QStringList lines{labels[13], "Ctrl+Alt+Shift+O"};
            const int actions[] = {0, 2, 1, 4, 8, 9, 3, 12};
            for (int index = 0; index < 8; index++) lines.append(QString::number(index + 1) + "  " + labels[actions[index]]);
            lines.append(QCoreApplication::translate("StreamControls", "Esc: Close"));
            QCoreApplication::removeTranslator(&translator);
            TestRenderer renderer;
            Overlay::OverlayManager manager;
            manager.setOverlayRenderer(&renderer);
            manager.prepareCommandPanel(lines.join('\n'));
            QVERIFY2(manager.setCommandPanelVisible(true), qPrintable(catalog));
            SDL_Surface* surface = manager.getUpdatedOverlaySurface(Overlay::OverlayStatusUpdate);
            QVERIFY(surface);
            QVERIFY(surface->w <= 1024 && surface->h <= 2048);
            SDL_FreeSurface(surface);
        }
    }

    void persistentCommandPanel()
    {
        TestRenderer renderer;
        Overlay::OverlayManager manager;
        manager.prepareCommandPanel("Stream controls\n1 Disconnect\nEsc: Close");
        QVERIFY(!manager.setCommandPanelVisible(true));
        manager.setOverlayRenderer(&renderer);
        QVERIFY(manager.setCommandPanelVisible(true));
        QVERIFY(manager.isCommandPanelVisible());
        QVERIFY(manager.isOverlayEnabled(Overlay::OverlayStatusUpdate));
        SDL_Surface* surface = manager.getUpdatedOverlaySurface(Overlay::OverlayStatusUpdate);
        QVERIFY(surface);
        SDL_FreeSurface(surface);
        const int updates = renderer.updates;
        QVERIFY(manager.setCommandPanelVisible(true));
        QCOMPARE(renderer.updates, updates);
        manager.showNotification("Clipboard sent to host", 100);
        QCOMPARE(QString::fromUtf8(manager.getOverlayText(Overlay::OverlayStatusUpdate)),
                 QString("Stream controls\n1 Disconnect\nEsc: Close"));
        manager.expireNotification(10000);
        QVERIFY(manager.isCommandPanelVisible());
        manager.updateOverlayText(Overlay::OverlayStatusUpdate, "Connection warning");
        manager.setOverlayState(Overlay::OverlayStatusUpdate, true);
        QVERIFY(!manager.isCommandPanelVisible());
        QVERIFY(!manager.setCommandPanelVisible(true));
        QCOMPARE(QString::fromUtf8(manager.getOverlayText(Overlay::OverlayStatusUpdate)), QString("Connection warning"));
        manager.setOverlayState(Overlay::OverlayStatusUpdate, false);
        QVERIFY(!manager.isCommandPanelVisible());
        QVERIFY(manager.setCommandPanelVisible(true));
        manager.setCommandPanelVisible(false);
        QVERIFY(!manager.isOverlayEnabled(Overlay::OverlayStatusUpdate));
    }

    void unicodeNotices_data()
    {
        QTest::addColumn<QString>("text");
        QTest::newRow("polish") << QString::fromUtf8("Schowek wysłano do komputera-gospodarza: zażółć gęślą jaźń");
        QTest::newRow("arabic-script") << QString::fromUtf8("تم إرسال الحافظة إلى المضيف");
        QTest::newRow("cjk") << QString::fromUtf8("剪贴板已发送到主机");
        QTest::newRow("tamil") << QString::fromUtf8("கிளிப்போர்டு அனுப்பப்பட்டது");
    }

    void unicodeNotices()
    {
        QFETCH(QString, text);
        TestRenderer renderer;
        Overlay::OverlayManager manager;
        manager.setOverlayRenderer(&renderer);
        manager.prepareNotifications({text});
        QCOMPARE(renderer.updates, 0);
        QVERIFY(!manager.isOverlayEnabled(Overlay::OverlayStatusUpdate));
        QElapsedTimer rendering;
        rendering.start();
        manager.showNotification(text, 100);
        qInfo() << "Unicode notice CPU render milliseconds:" << rendering.elapsed();
        QCOMPARE(QString::fromUtf8(manager.getOverlayText(Overlay::OverlayStatusUpdate)), text);
        SDL_Surface* surface = manager.getUpdatedOverlaySurface(Overlay::OverlayStatusUpdate);
        QVERIFY(surface != nullptr);
        QVERIFY(surface->w > 0 && surface->w <= 1024);
        QVERIFY(surface->h > 0 && surface->h <= 2048);
        const QByteArray pixels(static_cast<const char*>(surface->pixels), surface->pitch * surface->h);
        QVERIFY(pixels != QByteArray(pixels.size(), '\0'));
        int lightPixels = 0;
        for (int row = 0; row < surface->h; row++) {
            for (int column = 0; column < surface->w; column++) {
                const auto pixel = reinterpret_cast<const Uint32*>(static_cast<const char*>(surface->pixels) + row * surface->pitch)[column];
                Uint8 red, green, blue, alpha;
                SDL_GetRGBA(pixel, surface->format, &red, &green, &blue, &alpha);
                if (red > 200 && green > 200 && blue > 200 && alpha > 200) lightPixels++;
            }
        }
        QVERIFY(lightPixels > 20);
        SDL_FreeSurface(surface);
        manager.showNotification(text, 200);
        QVERIFY(manager.getUpdatedOverlaySurface(Overlay::OverlayStatusUpdate) == nullptr);
    }

    void cachedSurfaceOutlivesManager()
    {
        SDL_Surface* surface;
        QByteArray pixels;
        TestRenderer renderer;
        {
            Overlay::OverlayManager manager;
            manager.setOverlayRenderer(&renderer);
            manager.prepareNotifications({"Clipboard sent to host"});
            manager.showNotification("Clipboard sent to host", 100);
            surface = manager.getUpdatedOverlaySurface(Overlay::OverlayStatusUpdate);
            QVERIFY(surface != nullptr);
            pixels = QByteArray(static_cast<const char*>(surface->pixels), surface->pitch * surface->h);
        }
        QCOMPARE(QByteArray(static_cast<const char*>(surface->pixels), surface->pitch * surface->h), pixels);
        SDL_FreeSurface(surface);
    }

    void deadlineAndReplacement()
    {
        TestRenderer renderer;
        Overlay::OverlayManager manager;
        manager.setOverlayRenderer(&renderer);
        manager.showNotification("Clipboard sent to host", 100);
        QVERIFY(manager.isOverlayEnabled(Overlay::OverlayStatusUpdate));
        QCOMPARE(QString::fromUtf8(manager.getOverlayText(Overlay::OverlayStatusUpdate)), QString("Clipboard sent to host"));
        QCOMPARE(manager.getOverlayFontSize(Overlay::OverlayStatusUpdate), 24);
        QCOMPARE(manager.notificationTimeout(1000, 5000), 100);
        manager.expireNotification(5099);
        QVERIFY(manager.isOverlayEnabled(Overlay::OverlayStatusUpdate));
        manager.showNotification("Clipboard received from host", 5099);
        manager.expireNotification(5100);
        QVERIFY(manager.isOverlayEnabled(Overlay::OverlayStatusUpdate));
        manager.expireNotification(10099);
        QVERIFY(!manager.isOverlayEnabled(Overlay::OverlayStatusUpdate));
        QCOMPARE(manager.notificationTimeout(1000, 10099), 1000);
        QCOMPARE(renderer.updates, 3);
    }

    void repeatedTextRefreshesWithoutRendering()
    {
        TestRenderer renderer;
        Overlay::OverlayManager manager;
        manager.setOverlayRenderer(&renderer);
        manager.showNotification("Clipboard sent", 100);
        manager.showNotification("Clipboard sent", 200);
        QCOMPARE(renderer.updates, 1);
        manager.expireNotification(5100);
        QVERIFY(manager.isOverlayEnabled(Overlay::OverlayStatusUpdate));
        manager.expireNotification(5200);
        QVERIFY(!manager.isOverlayEnabled(Overlay::OverlayStatusUpdate));
        QCOMPARE(renderer.updates, 2);
    }

    void warningPriorityAndExpiry()
    {
        TestRenderer renderer;
        Overlay::OverlayManager manager;
        manager.setOverlayRenderer(&renderer);
        manager.updateOverlayText(Overlay::OverlayStatusUpdate, "Connection warning");
        manager.setOverlayState(Overlay::OverlayStatusUpdate, true);
        manager.showNotification("Clipboard sent", 100);
        QCOMPARE(QString::fromUtf8(manager.getOverlayText(Overlay::OverlayStatusUpdate)), QString("Connection warning"));
        QCOMPARE(manager.getOverlayFontSize(Overlay::OverlayStatusUpdate), 36);
        manager.expireNotification(5100);
        QVERIFY(manager.isOverlayEnabled(Overlay::OverlayStatusUpdate));
        QCOMPARE(QString::fromUtf8(manager.getOverlayText(Overlay::OverlayStatusUpdate)), QString("Connection warning"));
        manager.setOverlayState(Overlay::OverlayStatusUpdate, false);
        QVERIFY(!manager.isOverlayEnabled(Overlay::OverlayStatusUpdate));
        manager.showNotification("New clipboard notice", 6000);
        manager.updateOverlayText(Overlay::OverlayStatusUpdate, "Mouse mode");
        manager.setOverlayState(Overlay::OverlayStatusUpdate, true);
        manager.setOverlayState(Overlay::OverlayStatusUpdate, false);
        QVERIFY(manager.isOverlayEnabled(Overlay::OverlayStatusUpdate));
        QCOMPARE(QString::fromUtf8(manager.getOverlayText(Overlay::OverlayStatusUpdate)), QString("New clipboard notice"));
    }

    void surfaceAndTickWrap()
    {
        TestRenderer renderer;
        Overlay::OverlayManager manager;
        manager.setOverlayRenderer(&renderer);
        const Uint32 start = Uint32(-1000);
        manager.showNotification("Clipboard sent", start);
        auto* surface = manager.getUpdatedOverlaySurface(Overlay::OverlayStatusUpdate);
        QVERIFY(surface != nullptr);
        QVERIFY(surface->w > 0 && surface->h > 0);
        SDL_FreeSurface(surface);
        QCOMPARE(manager.notificationTimeout(1000, 3900), 100);
        manager.expireNotification(3999);
        QVERIFY(manager.isOverlayEnabled(Overlay::OverlayStatusUpdate));
        manager.expireNotification(4000);
        QVERIFY(!manager.isOverlayEnabled(Overlay::OverlayStatusUpdate));
        QVERIFY(manager.getUpdatedOverlaySurface(Overlay::OverlayStatusUpdate) == nullptr);
    }
};

QTEST_MAIN(OverlayNotificationsTest)
#include "tst_overlaynotifications.moc"
