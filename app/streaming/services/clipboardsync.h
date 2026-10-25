#pragma once

#include "sessionservices.h"
#include <QElapsedTimer>
#include <optional>

class ClipboardSync
{
public:
    enum class Feedback { None, Sent, Received, Failed, Denied, Unsupported, Skipped, Invalid };
    Feedback takeFeedback();
    explicit ClipboardSync(SessionServices& services, bool automatic = false);
    bool send(QByteArray text);
    void receive();
    void observeLocal(const QByteArray& text);
    std::optional<QByteArray> complete(const SessionServiceMessage& message);
    void dispatch();
    bool blocked() const;
    bool receiving() const;
    void invalidateLocal();
    void setFocused(bool focused);
    void disableAutomatic();
    bool automatic() const;

private:
    Feedback m_Feedback = Feedback::None;
    SessionServices& m_Services;
    QElapsedTimer m_Clock;
    quint64 m_Revision = 0;
    quint64 m_ReadRevision = 0;
    quint64 m_NextId = 1;
    quint64 m_ActiveId = 0;
    bool m_Sending = false;
    bool m_Automatic = false;
    bool m_Focused = true;
    bool m_LocalKnown = false;
    bool m_RemoteKnown = false;
    bool m_ReadExplicit = false;
    qint64 m_NextPoll = 1000;
    QByteArray m_RemoteHash;
    std::optional<QByteArray> m_ActiveWrite;
    bool m_ReceivePending = false;
    bool m_Blocked = false;
    qint64 m_RetryAt = 0;
    QByteArray m_LocalHash;
    std::optional<QByteArray> m_PendingWrite;
};
