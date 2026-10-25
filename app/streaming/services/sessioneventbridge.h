#pragma once

#include <QByteArray>
#include <QMutex>
#include <QVector>
#include <SDL.h>

struct SessionServiceMessage
{
    enum class Status { Ready, Success, Failed, TimedOut, TooLarge, ClipboardTick };
    quint64 generation = 0;
    quint64 requestId = 0;
    Status status = Status::Failed;
    QByteArray payload;
    int httpStatus = 0;
};

class SessionEventBridge
{
public:
    static constexpr int MaximumMessages = 16;
    static constexpr int MaximumPayload = 1024 * 1024;
    static constexpr int MaximumBytes = 4 * 1024 * 1024;

    explicit SessionEventBridge(quint64 generation);
    bool publish(SessionServiceMessage message);
    QVector<SessionServiceMessage> take(quint64 generation);
    void invalidate();
    bool active() const;
    static Uint32 eventType();

private:
    mutable QMutex m_Mutex;
    quint64 m_Generation;
    QVector<SessionServiceMessage> m_Messages;
    int m_Bytes = 0;
    bool m_WakeupPending = false;
};
