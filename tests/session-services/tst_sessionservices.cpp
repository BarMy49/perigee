#include <QtTest>
#include <QFile>
#include <QProcess>
#include <QSslKey>
#include <QSslSocket>
#include <QTcpServer>
#include <QTemporaryDir>
#include <thread>

#include "streaming/services/sessionservices.h"
#include "streaming/services/clipboardsync.h"
#include "streaming/services/clipboardtext.h"
#include "streaming/services/sdlclipboard.h"

class TlsFixtureServer : public QTcpServer
{
public:
    enum class Mode { Empty, Payload, Redirect, Oversized, Hold, Denied };
    QSslCertificate certificate;
    QSslKey key;
    QSslCertificate clientCertificate;
    Mode mode = Mode::Empty;
    int requests = 0;
    QByteArray lastPath;
    QByteArray lastBody;
    QByteArray lastMethod;

protected:
    void incomingConnection(qintptr descriptor) override
    {
        auto* socket = new QSslSocket(this);
        socket->setLocalCertificate(certificate);
        socket->setPrivateKey(key);
        auto tls = socket->sslConfiguration();
        tls.setCaCertificates({clientCertificate});
        tls.setPeerVerifyMode(QSslSocket::VerifyPeer);
        socket->setSslConfiguration(tls);
        QVERIFY(socket->setSocketDescriptor(descriptor));
        connect(socket, &QSslSocket::disconnected, socket, &QObject::deleteLater);
        connect(socket, &QSslSocket::readyRead, socket, [this, socket, buffer = QByteArray()]() mutable {
            buffer += socket->readAll();
            if (!buffer.contains("\r\n\r\n") || socket->property("responded").toBool()) return;
            const int headerEnd = buffer.indexOf("\r\n\r\n");
            int contentLength = 0;
            for (const auto& line : buffer.left(headerEnd).split('\n')) {
                if (line.toLower().startsWith("content-length:")) contentLength = line.mid(15).trimmed().toInt();
            }
            if (buffer.size() < headerEnd + 4 + contentLength) return;
            lastBody = buffer.mid(headerEnd + 4, contentLength);
            lastMethod = buffer.split(' ').value(0);
            socket->setProperty("responded", true);
            requests++;
            lastPath = buffer.split(' ').value(1);
            if (mode == Mode::Hold) return;
            if (mode == Mode::Denied) {
                socket->write("HTTP/1.1 401 Unauthorized\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
            }
            else if (mode == Mode::Redirect) {
                socket->write("HTTP/1.1 302 Found\r\nLocation: https://127.0.0.1:1/forbidden\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
            }
            else {
                const QByteArray body = mode == Mode::Payload ? QByteArray("synthetic fixture") :
                                        mode == Mode::Oversized ? QByteArray(SessionEventBridge::MaximumPayload + 1, 'x') : QByteArray();
                socket->write("HTTP/1.1 200 OK\r\nContent-Length: " + QByteArray::number(body.size()) + "\r\nConnection: close\r\n\r\n" + body);
            }
            socket->disconnectFromHost();
        });
        socket->startServerEncryption();
    }
};

class SessionServicesTest : public QObject
{
    Q_OBJECT
    QTemporaryDir m_Fixtures;
    QSslCertificate m_Certificate;
    QSslCertificate m_OtherCertificate;
    QSslKey m_Key;
    QSslKey m_OtherKey;

    SessionServiceSnapshot snapshot(quint16 port = 443) const
    {
        SessionServiceSnapshot value;
        value.enabled = true;
        value.hostUuid = "synthetic-host";
        value.endpoint = QUrl("https://127.0.0.1:" + QString::number(port));
        value.serverCertificate = m_Certificate;
        value.clientTls = QSslConfiguration::defaultConfiguration();
        value.clientTls.setLocalCertificate(m_Certificate);
        value.clientTls.setPrivateKey(m_Key);
        value.capabilities = ApolloCapabilities::parse("<root status_code=\"200\"><PairStatus>1</PairStatus><Permission>196608</Permission></root>", true);
        return value;
    }

    QVector<SessionServiceMessage> waitMessages(SessionServices& services)
    {
        QElapsedTimer timer;
        timer.start();
        while (timer.elapsed() < 20000) {
            SDL_Event event;
            if (SDL_WaitEventTimeout(&event, 50) && event.type == SessionEventBridge::eventType()) {
                auto messages = services.takeMessages();
                if (!messages.isEmpty()) return messages;
            }
        }
        return {};
    }

private slots:
    void initTestCase()
    {
        QVERIFY(m_Fixtures.isValid());
        QCOMPARE(SDL_Init(SDL_INIT_EVENTS), 0);
        for (const QString& name : {QString("host"), QString("other")}) {
            const QString base = m_Fixtures.path() + "/" + name;
            QProcess process;
            process.start("openssl", {"req", "-x509", "-newkey", "rsa:2048", "-nodes", "-days", "1", "-subj", "/CN=fixture", "-keyout", base + ".key", "-out", base + ".crt"});
            QVERIFY(process.waitForFinished());
            QCOMPARE(process.exitCode(), 0);
            QFile certificate(base + ".crt");
            QVERIFY(certificate.open(QIODevice::ReadOnly));
            if (name == "host") {
                m_Certificate = QSslCertificate(certificate.readAll());
                QFile key(base + ".key");
                QVERIFY(key.open(QIODevice::ReadOnly));
                m_Key = QSslKey(key.readAll(), QSsl::Rsa);
            }
            else {
                m_OtherCertificate = QSslCertificate(certificate.readAll());
                QFile key(base + ".key");
                QVERIFY(key.open(QIODevice::ReadOnly));
                m_OtherKey = QSslKey(key.readAll(), QSsl::Rsa);
            }
        }
        QVERIFY(!m_Certificate.isNull());
        QVERIFY(!m_Key.isNull());
    }

    void boundedOwnershipAndWakeup()
    {
        SessionEventBridge bridge(9);
        QVERIFY(SessionEventBridge::eventType() != SDL_USEREVENT);
        SDL_Event upstream = {};
        upstream.type = SDL_USEREVENT;
        upstream.user.code = 123;
        QCOMPARE(SDL_PushEvent(&upstream), 1);
        QVERIFY(!bridge.publish({8, 0, SessionServiceMessage::Status::Success, "stale"}));
        QByteArray data("owned");
        QVERIFY(bridge.publish({9, 1, SessionServiceMessage::Status::Success, data}));
        data.fill('x');
        for (int index = 1; index < SessionEventBridge::MaximumMessages; index++) {
            QVERIFY(bridge.publish({9, quint64(index + 1), SessionServiceMessage::Status::Success, "value"}));
        }
        QVERIFY(!bridge.publish({9, 99, SessionServiceMessage::Status::Success, "overflow"}));
        SDL_Event event;
        QCOMPARE(SDL_PeepEvents(&event, 1, SDL_GETEVENT, SessionEventBridge::eventType(), SessionEventBridge::eventType()), 1);
        QVERIFY(event.user.data1 == nullptr && event.user.data2 == nullptr);
        QCOMPARE(SDL_PeepEvents(&event, 1, SDL_GETEVENT, SessionEventBridge::eventType(), SessionEventBridge::eventType()), 0);
        QVERIFY(bridge.take(8).isEmpty());
        const auto messages = bridge.take(9);
        QCOMPARE(messages.size(), SessionEventBridge::MaximumMessages);
        QCOMPARE(messages.first().payload, QByteArray("owned"));
        bridge.invalidate();
        QVERIFY(!bridge.active());
        QVERIFY(!bridge.publish({9, 1, SessionServiceMessage::Status::Success, "late"}));
        QVERIFY(bridge.take(9).isEmpty());
        QCOMPARE(SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_USEREVENT, SDL_USEREVENT), 1);
        QCOMPARE(event.user.code, 123);
    }

    void byteLimitsAndFilteredWakeups()
    {
        SessionEventBridge bridge(10);
        QVERIFY(!bridge.publish({10, 1, SessionServiceMessage::Status::Success, QByteArray(SessionEventBridge::MaximumPayload + 1, 'x')}));
        for (int index = 0; index < SessionEventBridge::MaximumBytes / SessionEventBridge::MaximumPayload; index++) {
            QVERIFY(bridge.publish({10, 1, SessionServiceMessage::Status::Success, QByteArray(SessionEventBridge::MaximumPayload, 'x')}));
        }
        QVERIFY(!bridge.publish({10, 1, SessionServiceMessage::Status::Success, "x"}));
        bridge.invalidate();
        SessionEventBridge filtered(11);
        SDL_SetEventFilter([](void*, SDL_Event* event) { return event->type == SessionEventBridge::eventType() ? 0 : 1; }, nullptr);
        const bool accepted = filtered.publish({11, 1, SessionServiceMessage::Status::Success, "discard"});
        SDL_SetEventFilter(nullptr, nullptr);
        QVERIFY(!accepted);
        QVERIFY(filtered.take(11).isEmpty());
        filtered.invalidate();
    }

    void cancellationRacesProducer()
    {
        auto bridge = std::make_shared<SessionEventBridge>(15);
        std::thread producer([bridge]() {
            for (int index = 0; index < 10000; index++) {
                bridge->publish({15, quint64(index), SessionServiceMessage::Status::Success, "fixture"});
            }
        });
        bridge->invalidate();
        producer.join();
        QVERIFY(bridge->take(15).isEmpty());
        QVERIFY(!bridge->active());
        SDL_Event event;
        QCOMPARE(SDL_PeepEvents(&event, 1, SDL_GETEVENT, SessionEventBridge::eventType(), SessionEventBridge::eventType()), 0);
    }

    void disabledAndInvalidStart()
    {
        SessionServiceSnapshot disabled;
        SessionServices services(disabled);
        QVERIFY(!services.start());
        QVERIFY(!services.running());
        QCOMPARE(services.generation(), quint64(0));
        QVERIFY(!services.request("/fixture", 1));
        services.stop();
        for (int index = 0; index < 8; index++) {
            auto value = snapshot();
            if (index == 0) value.capabilities = {};
            if (index == 1) value.endpoint.setScheme("http");
            if (index == 2) value.serverCertificate = QSslCertificate();
            if (index == 3) value.clientTls.setPrivateKey({});
            if (index == 4) value.endpoint.setQuery("secret=fixture");
            if (index == 5) value.endpoint.setUserInfo("fixture");
            if (index == 6) value.endpoint.setPort(0);
            if (index == 7) value.hostUuid.clear();
            SessionServices invalid(value);
            QVERIFY(!invalid.start());
            QVERIFY(!invalid.running());
        }
    }

    void workerRunsWithoutGuiEvents()
    {
        SessionServices services(snapshot());
        QVERIFY(services.start());
        QVERIFY(!services.start());
        const auto messages = waitMessages(services);
        QCOMPARE(messages.size(), 1);
        QCOMPARE(messages.first().status, SessionServiceMessage::Status::Ready);
        QCOMPARE(messages.first().generation, services.generation());
        QVERIFY(!services.request("https://elsewhere/fixture", 1));
        QVERIFY(!services.request("//elsewhere", 1));
        QVERIFY(!services.request("/fixture?secret", 1));
        QVERIFY(!services.request("/fixture", 0));
        QVERIFY(!services.request("/fixture", 1, 0));
        services.stop();
        QVERIFY(!services.running());
        QVERIFY(services.takeMessages().isEmpty());
    }

    void transportResults_data()
    {
        QTest::addColumn<int>("mode");
        QTest::addColumn<int>("status");
        QTest::newRow("empty-success") << int(TlsFixtureServer::Mode::Empty) << int(SessionServiceMessage::Status::Success);
        QTest::newRow("owned-payload") << int(TlsFixtureServer::Mode::Payload) << int(SessionServiceMessage::Status::Success);
        QTest::newRow("redirect-refused") << int(TlsFixtureServer::Mode::Redirect) << int(SessionServiceMessage::Status::Failed);
        QTest::newRow("bounded-receive") << int(TlsFixtureServer::Mode::Oversized) << int(SessionServiceMessage::Status::TooLarge);
        QTest::newRow("timeout") << int(TlsFixtureServer::Mode::Hold) << int(SessionServiceMessage::Status::TimedOut);
    }

    void transportResults()
    {
        QFETCH(int, mode);
        QFETCH(int, status);
        TlsFixtureServer server;
        server.certificate = m_Certificate;
        server.key = m_Key;
        server.clientCertificate = m_Certificate;
        server.mode = TlsFixtureServer::Mode(mode);
        QVERIFY(server.listen(QHostAddress::LocalHost));
        auto value = snapshot(server.serverPort());
        SessionServices services(value);
        value.endpoint.setHost("invalid.fixture");
        value.serverCertificate = m_OtherCertificate;
        QVERIFY(services.start());
        QCOMPARE(waitMessages(services).size(), 1);
        QVERIFY(services.request("/fixture", 42, mode == int(TlsFixtureServer::Mode::Hold) ? 5000 : 10000));
        QVERIFY(!services.request("/fixture", 43));
        QTRY_COMPARE_WITH_TIMEOUT(server.requests, 1, 20000);
        QVector<SessionServiceMessage> messages;
        if (mode == int(TlsFixtureServer::Mode::Hold)) messages = waitMessages(services);
        else QTRY_VERIFY(!messages.isEmpty() || !(messages = services.takeMessages()).isEmpty());
        QCOMPARE(messages.size(), 1);
        QCOMPARE(messages.first().requestId, quint64(42));
        QCOMPARE(int(messages.first().status), status);
        QCOMPARE(server.lastPath, QByteArray("/fixture"));
        QCOMPARE(messages.first().payload, mode == int(TlsFixtureServer::Mode::Payload) ? QByteArray("synthetic fixture") : QByteArray());
        services.stop();
    }

    void wrongPinAndConnectionFailure()
    {
        TlsFixtureServer server;
        server.certificate = m_OtherCertificate;
        server.key = m_OtherKey;
        server.clientCertificate = m_Certificate;
        QVERIFY(server.listen(QHostAddress::LocalHost));
        SessionServices services(snapshot(server.serverPort()));
        QVERIFY(services.start());
        QCOMPARE(waitMessages(services).size(), 1);
        QVERIFY(services.request("/fixture", 1));
        QVector<SessionServiceMessage> messages;
        QTRY_VERIFY(!messages.isEmpty() || !(messages = services.takeMessages()).isEmpty());
        QCOMPARE(messages.first().status, SessionServiceMessage::Status::Failed);
        QCOMPARE(server.requests, 0);
        services.stop();
        const auto port = server.serverPort();
        server.close();
        SessionServices refused(snapshot(port));
        QVERIFY(refused.start());
        QCOMPARE(waitMessages(refused).size(), 1);
        QVERIFY(refused.request("/fixture", 2));
        const auto failure = waitMessages(refused);
        QCOMPARE(failure.size(), 1);
        QCOMPARE(failure.first().status, SessionServiceMessage::Status::Failed);
    }

    void rapidStartFailureAndReconnect()
    {
        SessionServices services(snapshot());
        quint64 previous = 0;
        for (int index = 0; index < 200; index++) {
            QVERIFY(services.start());
            QVERIFY(services.generation() > previous);
            previous = services.generation();
            services.stop();
            QVERIFY(!services.running());
            QVERIFY(services.takeMessages().isEmpty());
        }
        QVERIFY(services.start());
        const auto messages = waitMessages(services);
        QCOMPARE(messages.size(), 1);
        QCOMPARE(messages.first().generation, services.generation());
        services.stop();
    }

    void shutdownWithReplyPending()
    {
        TlsFixtureServer server;
        server.certificate = m_Certificate;
        server.key = m_Key;
        server.clientCertificate = m_Certificate;
        server.mode = TlsFixtureServer::Mode::Hold;
        QVERIFY(server.listen(QHostAddress::LocalHost));
        quint64 previous = 0;
        for (int index = 0; index < 30; index++) {
            SessionServices services(snapshot(server.serverPort()));
            QVERIFY(services.start());
            QVERIFY(services.generation() > previous);
            previous = services.generation();
            QCOMPARE(waitMessages(services).size(), 1);
            QVERIFY(services.request("/fixture", quint64(index + 1), 30000));
            QTRY_COMPARE(server.requests, index + 1);
            services.stop();
            QVERIFY(services.takeMessages().isEmpty());
            QVERIFY(!services.running());
        }
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }

    void clipboardPlatformGate()
    {
#ifdef Q_OS_LINUX
        QVERIFY(SdlClipboard::automaticSupported("wayland", "GNOME"));
        QVERIFY(SdlClipboard::automaticSupported("wayland", "GNOME:GNOME-Classic"));
#endif
        QVERIFY(!SdlClipboard::automaticSupported("x11", "GNOME"));
        QVERIFY(!SdlClipboard::automaticSupported("wayland", "KDE"));
        QVERIFY(!SdlClipboard::automaticSupported("wayland", "NOTGNOME"));
        QVERIFY(!SdlClipboard::automaticSupported("wayland", ""));
    }

    void clipboardTicksCoalesce()
    {
        SessionEventBridge bridge(91);
        for (int index = 0; index < 100; index++) {
            QVERIFY(bridge.publish({91, 0, SessionServiceMessage::Status::ClipboardTick, {}}));
        }
        QVERIFY(!bridge.publish({91, 1, SessionServiceMessage::Status::ClipboardTick, {}}));
        QVERIFY(!bridge.publish({91, 0, SessionServiceMessage::Status::ClipboardTick, "invalid"}));
        QVERIFY(bridge.publish({91, 12, SessionServiceMessage::Status::Success, "completion"}));
        const auto messages = bridge.take(91);
        QCOMPARE(messages.size(), 2);
        QCOMPARE(messages.first().status, SessionServiceMessage::Status::ClipboardTick);
        QCOMPARE(messages.last().requestId, quint64(12));
        bridge.invalidate();
        QVERIFY(!bridge.publish({91, 0, SessionServiceMessage::Status::ClipboardTick, {}}));
    }

    void clipboardWorkerTicksWithoutGuiLoop()
    {
        auto value = snapshot();
        value.capabilities = ApolloCapabilities::parse("<root status_code=\"200\"><PairStatus>1</PairStatus><Permission>117637120</Permission></root>", true);
        SessionServices services(value);
        QVERIFY(services.start());
        QCOMPARE(waitMessages(services).first().status, SessionServiceMessage::Status::Ready);
        services.setClipboardPolling(true);
        auto messages = waitMessages(services);
        QCOMPARE(messages.size(), 1);
        QCOMPARE(messages.first().status, SessionServiceMessage::Status::ClipboardTick);
        QCOMPARE(messages.first().generation, services.generation());
        services.setClipboardPolling(false);
        QThread::msleep(1200);
        QVERIFY(services.takeMessages().isEmpty());
        services.setClipboardPolling(true);
        services.stop();
        QVERIFY(services.takeMessages().isEmpty());
    }

    void clipboardAutomaticFocusAndFallback()
    {
        TlsFixtureServer server;
        server.certificate = m_Certificate;
        server.clientCertificate = m_Certificate;
        server.key = m_Key;
        QVERIFY(server.listen(QHostAddress::LocalHost));
        auto value = snapshot(server.serverPort());
        value.capabilities = ApolloCapabilities::parse("<root status_code=\"200\"><PairStatus>1</PairStatus><Permission>117637120</Permission></root>", true);
        SessionServices services(value);
        QVERIFY(services.start());
        QCOMPARE(waitMessages(services).size(), 1);
        ClipboardSync automatic(services, true);
        automatic.observeLocal("initial local");
        QTest::qWait(1050);
        automatic.setFocused(false);
        automatic.observeLocal("copied while unfocused");
        automatic.dispatch();
        QCOMPARE(server.requests, 0);
        automatic.setFocused(true);
        automatic.dispatch();
        QTRY_COMPARE(server.requests, 1);
        auto messages = waitMessages(services);
        QCOMPARE(messages.size(), 1);
        QVERIFY(!automatic.complete(messages.first()).has_value());
        QCOMPARE(server.lastMethod, QByteArray("GET"));
        automatic.observeLocal("local change");
        automatic.observeLocal("latest local change");
        automatic.dispatch();
        QTRY_COMPARE(server.requests, 2);
        messages = waitMessages(services);
        QCOMPARE(messages.size(), 1);
        automatic.complete(messages.first());
        QCOMPARE(server.lastMethod, QByteArray("POST"));
        QCOMPARE(server.lastBody, QByteArray("latest local change"));
        server.mode = TlsFixtureServer::Mode::Payload;
        QTest::qWait(1050);
        automatic.dispatch();
        QTRY_COMPARE(server.requests, 3);
        messages = waitMessages(services);
        QCOMPARE(messages.size(), 1);
        automatic.setFocused(false);
        automatic.setFocused(true);
        QVERIFY(!automatic.complete(messages.first()).has_value());
        QTest::qWait(1050);
        automatic.dispatch();
        QTRY_COMPARE(server.requests, 4);
        messages = waitMessages(services);
        QCOMPARE(messages.size(), 1);
        const auto recovered = automatic.complete(messages.first());
        QVERIFY(recovered.has_value());
        QCOMPARE(*recovered, QByteArray("synthetic fixture"));
        automatic.observeLocal(QByteArray(SessionEventBridge::MaximumPayload + 1, 'x'));
        automatic.observeLocal("pending automatic change");
        automatic.disableAutomatic();
        QVERIFY(!automatic.automatic());
        QTest::qWait(1050);
        automatic.dispatch();
        QCOMPARE(server.requests, 4);
        QVERIFY(automatic.send("manual fallback"));
        QTRY_COMPARE(server.requests, 5);
        messages = waitMessages(services);
        QCOMPARE(messages.size(), 1);
        automatic.complete(messages.first());
        QCOMPARE(server.lastBody, QByteArray("manual fallback"));
        services.stop();
    }

    void clipboardAutomaticDirectionPermissions_data()
    {
        QTest::addColumn<bool>("sendAllowed");
        QTest::newRow("receive-only") << false;
        QTest::newRow("send-only") << true;
    }

    void clipboardAutomaticDirectionPermissions()
    {
        QFETCH(bool, sendAllowed);
        TlsFixtureServer server;
        server.certificate = m_Certificate;
        server.clientCertificate = m_Certificate;
        server.key = m_Key;
        QVERIFY(server.listen(QHostAddress::LocalHost));
        auto value = snapshot(server.serverPort());
        const quint32 direction = sendAllowed ? ApolloCapabilities::ClipboardSet : ApolloCapabilities::ClipboardRead;
        value.capabilities = ApolloCapabilities::parse(QString("<root status_code=\"200\"><PairStatus>1</PairStatus><Permission>%1</Permission></root>").arg(ApolloCapabilities::View | direction), true);
        SessionServices services(value);
        QVERIFY(services.start());
        QCOMPARE(waitMessages(services).size(), 1);
        ClipboardSync automatic(services, true);
        automatic.observeLocal("initial");
        QTest::qWait(1050);
        automatic.observeLocal("changed local");
        automatic.dispatch();
        QTRY_COMPARE(server.requests, 1);
        auto messages = waitMessages(services);
        QCOMPARE(messages.size(), 1);
        automatic.complete(messages.first());
        QCOMPARE(server.lastMethod, QByteArray(sendAllowed ? "POST" : "GET"));
        if (sendAllowed) {
            automatic.receive();
            automatic.dispatch();
            QCOMPARE(server.requests, 1);
        }
        else {
            QVERIFY(!automatic.send("forbidden upload"));
            automatic.dispatch();
            QCOMPARE(server.requests, 1);
        }
        services.stop();
    }

    void clipboardProtocolAndState()
    {
        TlsFixtureServer server;
        server.certificate = m_Certificate;
        server.clientCertificate = m_Certificate;
        server.key = m_Key;
        QVERIFY(server.listen(QHostAddress::LocalHost));
        auto value = snapshot(server.serverPort());
        value.capabilities = ApolloCapabilities::parse("<root status_code=\"200\"><PairStatus>1</PairStatus><Permission>117637120</Permission></root>", true);
        SessionServices services(value);
        QVERIFY(services.start());
        QCOMPARE(waitMessages(services).size(), 1);
        QVERIFY(!services.clipboardRequest(true, QByteArray("\xff", 1), 999));
        ClipboardSync clipboard(services);
        clipboard.observeLocal("initial contents must not upload");
        clipboard.dispatch();
        QCOMPARE(server.requests, 0);
        QVERIFY(!clipboard.send(QByteArray(1024 * 1024 + 1, 'x')));
        QVERIFY(!clipboard.send(QByteArray("a\0b", 3)));
        QVERIFY(!clipboard.send(QByteArray("\xff", 1)));
        QVERIFY(!clipboard.send(QByteArray("\xc3", 1)));
        QVERIFY(ClipboardText::valid(QByteArray("\xef\xbb\xbftext")));
        clipboard.receive();
        QTRY_COMPARE_WITH_TIMEOUT(server.requests, 1, 20000);
        auto messages = waitMessages(services);
        QCOMPARE(messages.size(), 1);
        QCOMPARE(server.lastPath, QByteArray("/actions/clipboard?type=text"));
        QCOMPARE(messages.first().httpStatus, 200);
        QCOMPARE(clipboard.takeFeedback(), ClipboardSync::Feedback::None);
        const auto empty = clipboard.complete(messages.first());
        QCOMPARE(clipboard.takeFeedback(), ClipboardSync::Feedback::Received);
        QCOMPARE(clipboard.takeFeedback(), ClipboardSync::Feedback::None);
        QVERIFY(empty.has_value());
        QVERIFY(empty->isEmpty());
        clipboard.receive();
        clipboard.invalidateLocal();
        QTRY_COMPARE(server.requests, 2);
        messages = waitMessages(services);
        QCOMPARE(messages.size(), 1);
        QVERIFY(!clipboard.complete(messages.first()).has_value());
        QCOMPARE(clipboard.takeFeedback(), ClipboardSync::Feedback::Skipped);
        QVERIFY(clipboard.send("first"));
        QVERIFY(clipboard.send("second"));
        QVERIFY(clipboard.send("latest\r\nUTF-8: \xc3\xa9"));
        QTRY_COMPARE(server.requests, 3);
        messages = waitMessages(services);
        QCOMPARE(messages.size(), 1);
        QVERIFY(!clipboard.complete(messages.first()).has_value());
        QCOMPARE(clipboard.takeFeedback(), ClipboardSync::Feedback::Sent);
        clipboard.dispatch();
        QTRY_COMPARE(server.requests, 4);
        messages = waitMessages(services);
        QCOMPARE(messages.size(), 1);
        QCOMPARE(server.lastMethod, QByteArray("POST"));
        QCOMPARE(server.lastBody, QByteArray("latest\r\nUTF-8: \xc3\xa9"));
        clipboard.complete(messages.first());
        ClipboardSync automatic(services, true);
        automatic.observeLocal("preexisting local clipboard");
        automatic.dispatch();
        QCOMPARE(server.requests, 4);
        QTest::qWait(1050);
        automatic.dispatch();
        QTRY_COMPARE(server.requests, 5);
        messages = waitMessages(services);
        QCOMPARE(messages.size(), 1);
        QVERIFY(!automatic.complete(messages.first()).has_value());
        server.mode = TlsFixtureServer::Mode::Payload;
        QTest::qWait(1050);
        automatic.dispatch();
        QTRY_COMPARE(server.requests, 6);
        messages = waitMessages(services);
        const auto remote = automatic.complete(messages.first());
        QVERIFY(remote.has_value());
        QCOMPARE(*remote, QByteArray("synthetic fixture"));
        automatic.observeLocal(*remote);
        automatic.dispatch();
        QCOMPARE(server.requests, 6);
        QCOMPARE(server.lastMethod, QByteArray("GET"));
        server.mode = TlsFixtureServer::Mode::Denied;
        clipboard.receive();
        QTRY_COMPARE(server.requests, 7);
        messages = waitMessages(services);
        QCOMPARE(messages.size(), 1);
        QCOMPARE(messages.first().httpStatus, 401);
        QVERIFY(!clipboard.complete(messages.first()).has_value());
        QVERIFY(clipboard.blocked());
        QCOMPARE(clipboard.takeFeedback(), ClipboardSync::Feedback::Denied);
        QVERIFY(!clipboard.send("must not send"));
        clipboard.receive();
        clipboard.dispatch();
        QCOMPARE(server.requests, 7);
        services.stop();
    }

    void cleanupTestCase()
    {
        SDL_Quit();
    }
};

QTEST_GUILESS_MAIN(SessionServicesTest)
#include "tst_sessionservices.moc"
