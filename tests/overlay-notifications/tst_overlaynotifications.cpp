#include <QtTest>
#include <QFile>
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

QTEST_GUILESS_MAIN(OverlayNotificationsTest)
#include "tst_overlaynotifications.moc"
