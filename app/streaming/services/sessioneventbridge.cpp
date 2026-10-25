#include "sessioneventbridge.h"

#include <QMutexLocker>

SessionEventBridge::SessionEventBridge(quint64 generation) :
    m_Generation(generation)
{
}

Uint32 SessionEventBridge::eventType()
{
    static const Uint32 first = SDL_RegisterEvents(2);
    return first == Uint32(-1) ? Uint32(-1) : first + 1;
}

bool SessionEventBridge::publish(SessionServiceMessage message)
{
    QMutexLocker lock(&m_Mutex);
    if (m_Generation == 0 || message.generation != m_Generation ||
            message.payload.size() > MaximumPayload || eventType() == Uint32(-1)) return false;
    if (message.status == SessionServiceMessage::Status::ClipboardTick) {
        if (message.requestId != 0 || !message.payload.isEmpty()) return false;
        for (const auto& pending : m_Messages) {
            if (pending.status == SessionServiceMessage::Status::ClipboardTick) return true;
        }
    }
    if (m_Messages.size() >= MaximumMessages || message.payload.size() > MaximumBytes - m_Bytes) {
        return false;
    }
    if (!m_WakeupPending) {
        SDL_Event event = {};
        event.type = eventType();
        if (SDL_PushEvent(&event) != 1) {
            return false;
        }
        m_WakeupPending = true;
    }
    m_Bytes += message.payload.size();
    m_Messages.append(std::move(message));
    return true;
}

QVector<SessionServiceMessage> SessionEventBridge::take(quint64 generation)
{
    QMutexLocker lock(&m_Mutex);
    if (generation == 0 || generation != m_Generation) {
        return {};
    }
    QVector<SessionServiceMessage> messages;
    messages.swap(m_Messages);
    m_Bytes = 0;
    m_WakeupPending = false;
    return messages;
}

void SessionEventBridge::invalidate()
{
    QMutexLocker lock(&m_Mutex);
    m_Generation = 0;
    m_Messages.clear();
    m_Bytes = 0;
    m_WakeupPending = false;
    if (eventType() != Uint32(-1)) {
        SDL_FlushEvent(eventType());
    }
}

bool SessionEventBridge::active() const
{
    QMutexLocker lock(&m_Mutex);
    return m_Generation != 0;
}
