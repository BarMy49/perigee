#include "sessionservices.h"
#include "clipboardtext.h"

#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QSslError>
#include <QSslKey>
#include <QTimer>
#include <atomic>

class SessionServiceWorker : public QObject
{
    Q_OBJECT

public:
    SessionServiceWorker(SessionServiceSnapshot snapshot, quint64 generation,
                         std::shared_ptr<SessionEventBridge> bridge) :
        m_Snapshot(std::move(snapshot)), m_Generation(generation), m_Bridge(std::move(bridge))
    {
    }

    void initialize()
    {
        Q_ASSERT(QThread::currentThread() == thread());
        m_Network = new QNetworkAccessManager(this);
        m_Network->setProxy(QNetworkProxy::NoProxy);
        m_Timeout = new QTimer(this);
        m_Timeout->setSingleShot(true);
        m_ClipboardTimer = new QTimer(this);
        m_ClipboardTimer->setInterval(1000);
        connect(m_ClipboardTimer, &QTimer::timeout, this, [this]() {
            m_Bridge->publish({m_Generation, 0, SessionServiceMessage::Status::ClipboardTick, {}});
        });
        connect(m_Timeout, &QTimer::timeout, this, [this]() {
            finish(SessionServiceMessage::Status::TimedOut);
        });
        m_Bridge->publish({m_Generation, 0, SessionServiceMessage::Status::Ready, {}});
    }

    void request(QString path, quint64 requestId, int timeoutMs, bool clipboard = false, bool send = false, QByteArray payload = {})
    {
        Q_ASSERT(QThread::currentThread() == thread());
        if (!m_Bridge->active() || !m_Network || m_Reply) {
            return;
        }
        m_RequestId = requestId;
        m_Payload.clear();
        QUrl url = m_Snapshot.endpoint;
        url.setPath(path);
        if (clipboard) url.setQuery("type=text");
        QNetworkRequest request(url);
        auto tls = m_Snapshot.clientTls;
        tls.setPeerVerifyMode(QSslSocket::VerifyPeer);
        tls.setCaCertificates({m_Snapshot.serverCertificate});
        request.setSslConfiguration(tls);
        request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);
        if (send) {
            request.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain; charset=utf-8");
            m_Reply = m_Network->post(request, payload);
        }
        else {
            m_Reply = m_Network->get(request);
        }
        m_Reply->setReadBufferSize(SessionEventBridge::MaximumPayload + 1);
        connect(m_Reply, &QNetworkReply::sslErrors, this, [this](const QList<QSslError>& errors) {
            if (m_Reply->sslConfiguration().peerCertificate() != m_Snapshot.serverCertificate) {
                finish(SessionServiceMessage::Status::Failed);
                return;
            }
            for (const QSslError& error : errors) {
                if (error.error() != QSslError::SelfSignedCertificate &&
                        error.error() != QSslError::SelfSignedCertificateInChain &&
                        error.error() != QSslError::HostNameMismatch &&
                        error.error() != QSslError::UnableToGetLocalIssuerCertificate &&
                        error.error() != QSslError::UnableToVerifyFirstCertificate &&
                        error.error() != QSslError::CertificateUntrusted) {
                    finish(SessionServiceMessage::Status::Failed);
                    return;
                }
            }
            m_Reply->ignoreSslErrors(errors);
        });
        connect(m_Reply, &QNetworkReply::encrypted, this, [this]() {
            if (m_Reply->sslConfiguration().peerCertificate() != m_Snapshot.serverCertificate) {
                finish(SessionServiceMessage::Status::Failed);
            }
        });
        connect(m_Reply, &QIODevice::readyRead, this, [this]() { readAvailable(); });
        connect(m_Reply, &QNetworkReply::finished, this, [this]() {
            if (!readAvailable()) {
                return;
            }
            const int status = m_Reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            const bool success = m_Reply->error() == QNetworkReply::NoError && status >= 200 && status < 300 &&
                    m_Reply->sslConfiguration().peerCertificate() == m_Snapshot.serverCertificate;
            finish(success ? SessionServiceMessage::Status::Success : SessionServiceMessage::Status::Failed);
        });
        m_Timeout->start(timeoutMs);
    }

    void setClipboardPolling(bool enabled)
    {
        Q_ASSERT(QThread::currentThread() == thread());
        if (enabled && m_Bridge->active()) m_ClipboardTimer->start();
        else m_ClipboardTimer->stop();
    }

    void shutdown()
    {
        Q_ASSERT(QThread::currentThread() == thread());
        if (m_Timeout) {
            m_Timeout->stop();
        }
        if (m_ClipboardTimer) m_ClipboardTimer->stop();
        discardReply();
        QThread::currentThread()->quit();
    }

private:
    bool readAvailable()
    {
        if (!m_Reply) {
            return false;
        }
        const qint64 remaining = SessionEventBridge::MaximumPayload - m_Payload.size();
        m_Payload.append(m_Reply->read(remaining + 1));
        if (m_Payload.size() > SessionEventBridge::MaximumPayload) {
            finish(SessionServiceMessage::Status::TooLarge);
            return false;
        }
        return true;
    }

    void discardReply()
    {
        if (m_Reply) {
            auto* reply = m_Reply;
            m_Reply = nullptr;
            disconnect(reply, nullptr, this, nullptr);
            reply->abort();
            reply->deleteLater();
        }
        m_Payload.clear();
    }

    void finish(SessionServiceMessage::Status status)
    {
        m_Timeout->stop();
        const int httpStatus = m_Reply ? m_Reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() : 0;
        QByteArray payload;
        if (status == SessionServiceMessage::Status::Success) {
            payload.swap(m_Payload);
        }
        discardReply();
        m_Bridge->publish({m_Generation, m_RequestId, status, std::move(payload), httpStatus});
    }

    const SessionServiceSnapshot m_Snapshot;
    const quint64 m_Generation;
    const std::shared_ptr<SessionEventBridge> m_Bridge;
    QNetworkAccessManager* m_Network = nullptr;
    QTimer* m_Timeout = nullptr;
    QTimer* m_ClipboardTimer = nullptr;
    QNetworkReply* m_Reply = nullptr;
    quint64 m_RequestId = 0;
    QByteArray m_Payload;
};

SessionServices::SessionServices(SessionServiceSnapshot snapshot) :
    m_Snapshot(std::move(snapshot))
{
}

SessionServices::~SessionServices()
{
    stop();
}

bool SessionServices::start()
{
    Q_ASSERT(QThread::currentThread() == m_Thread.thread());
    if (m_Worker || !m_Snapshot.enabled) {
        return false;
    }
    const auto& endpoint = m_Snapshot.endpoint;
    if (m_Snapshot.hostUuid.isEmpty() || !m_Snapshot.capabilities.authenticated ||
            !endpoint.isValid() || endpoint.scheme() != "https" || endpoint.host().isEmpty() ||
            endpoint.authority().contains('@') || endpoint.hasQuery() || endpoint.hasFragment() ||
            endpoint.port(443) < 1 || endpoint.port(443) > 65535 ||
            m_Snapshot.serverCertificate.isNull() || m_Snapshot.clientTls.localCertificate().isNull() ||
            m_Snapshot.clientTls.privateKey().isNull() || SessionEventBridge::eventType() == Uint32(-1)) {
        return false;
    }
    static std::atomic<quint64> nextGeneration{1};
    m_Generation = nextGeneration.fetch_add(1);
    m_Bridge = std::make_shared<SessionEventBridge>(m_Generation);
    m_Worker = new SessionServiceWorker(m_Snapshot, m_Generation, m_Bridge);
    m_Worker->moveToThread(&m_Thread);
    QObject::connect(&m_Thread, &QThread::started, m_Worker, &SessionServiceWorker::initialize);
    QObject::connect(&m_Thread, &QThread::finished, m_Worker, &QObject::deleteLater);
    m_Thread.start();
    return true;
}

void SessionServices::stop()
{
    Q_ASSERT(QThread::currentThread() == m_Thread.thread());
    if (!m_Worker) {
        return;
    }
    m_Bridge->invalidate();
    QMetaObject::invokeMethod(m_Worker, &SessionServiceWorker::shutdown, Qt::QueuedConnection);
    m_Thread.wait();
    m_Worker = nullptr;
    m_Bridge.reset();
    m_Generation = 0;
    m_RequestPending = false;
}

bool SessionServices::request(const QString& path, quint64 requestId, int timeoutMs)
{
    Q_ASSERT(QThread::currentThread() == m_Thread.thread());
    static const QRegularExpression safePath("^/[A-Za-z0-9/_-]{1,127}$");
    if (!m_Worker || m_RequestPending || requestId == 0 || timeoutMs < 1 || timeoutMs > 30000 ||
            path.startsWith("//") || path.contains('\n') || !safePath.match(path).hasMatch()) {
        return false;
    }
    m_RequestPending = true;
    QMetaObject::invokeMethod(m_Worker, [worker = m_Worker, path, requestId, timeoutMs]() {
        worker->request(path, requestId, timeoutMs);
    }, Qt::QueuedConnection);
    return true;
}

bool SessionServices::clipboardAllowed(bool send) const
{
    const auto& capabilities = m_Snapshot.capabilities;
    const auto direction = send ? ApolloCapabilities::ClipboardSet : ApolloCapabilities::ClipboardRead;
    const bool view = capabilities.permission(ApolloCapabilities::View) == ApolloCapabilities::State::Available ||
            capabilities.permission(ApolloCapabilities::Launch) == ApolloCapabilities::State::Available;
    return m_Worker && view && capabilities.permission(direction) == ApolloCapabilities::State::Available;
}

bool SessionServices::clipboardRequest(bool send, QByteArray payload, quint64 requestId)
{
    Q_ASSERT(QThread::currentThread() == m_Thread.thread());
    if (!clipboardAllowed(send) || m_RequestPending || requestId == 0 ||
            !ClipboardText::valid(payload) ||
            (!send && !payload.isEmpty())) {
        return false;
    }
    m_RequestPending = true;
    QMetaObject::invokeMethod(m_Worker, [worker = m_Worker, send, payload = std::move(payload), requestId]() {
        worker->request("/actions/clipboard", requestId, 5000, true, send, payload);
    }, Qt::QueuedConnection);
    return true;
}

void SessionServices::setClipboardPolling(bool enabled)
{
    Q_ASSERT(QThread::currentThread() == m_Thread.thread());
    if (!m_Worker || (enabled && !clipboardAllowed(false) && !clipboardAllowed(true))) return;
    QMetaObject::invokeMethod(m_Worker, [worker = m_Worker, enabled]() {
        worker->setClipboardPolling(enabled);
    }, Qt::QueuedConnection);
}

QVector<SessionServiceMessage> SessionServices::takeMessages()
{
    Q_ASSERT(QThread::currentThread() == m_Thread.thread());
    if (!m_Bridge) {
        return {};
    }
    auto messages = m_Bridge->take(m_Generation);
    for (const auto& message : messages) {
        if (message.requestId != 0) {
            m_RequestPending = false;
        }
    }
    return messages;
}

quint64 SessionServices::generation() const
{
    return m_Generation;
}

bool SessionServices::running() const
{
    return m_Worker != nullptr;
}

#include "sessionservices.moc"
