#pragma once

#include "sessioneventbridge.h"
#include "backend/apollocapabilities.h"

#include <QSslConfiguration>
#include <QThread>
#include <memory>

struct SessionServiceSnapshot
{
    QString hostUuid;
    QUrl endpoint;
    QSslCertificate serverCertificate;
    QSslConfiguration clientTls;
    ApolloCapabilities capabilities;
    bool enabled = false;
};

class SessionServiceWorker;

class SessionServices
{
public:
    explicit SessionServices(SessionServiceSnapshot snapshot);
    ~SessionServices();
    SessionServices(const SessionServices&) = delete;
    SessionServices& operator=(const SessionServices&) = delete;

    bool start();
    void stop();
    bool request(const QString& path, quint64 requestId, int timeoutMs = 5000);
    bool clipboardAllowed(bool send) const;
    bool clipboardRequest(bool send, QByteArray payload, quint64 requestId);
    void setClipboardPolling(bool enabled);
    QVector<SessionServiceMessage> takeMessages();
    quint64 generation() const;
    bool running() const;

private:
    const SessionServiceSnapshot m_Snapshot;
    QThread m_Thread;
    SessionServiceWorker* m_Worker = nullptr;
    std::shared_ptr<SessionEventBridge> m_Bridge;
    quint64 m_Generation = 0;
    bool m_RequestPending = false;
};
