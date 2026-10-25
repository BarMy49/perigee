#include "clipboardsync.h"
#include "clipboardtext.h"

#include <QCryptographicHash>

ClipboardSync::ClipboardSync(SessionServices& services, bool automatic) : m_Services(services), m_Automatic(automatic)
{
    m_Clock.start();
}

void ClipboardSync::observeLocal(const QByteArray& text)
{
    if (m_Blocked || !ClipboardText::valid(text)) return;
    const auto hash = QCryptographicHash::hash(text, QCryptographicHash::Sha256);
    if (hash != m_LocalHash) {
        if (m_Automatic && m_Focused && m_LocalKnown && hash != m_RemoteHash && m_Services.clipboardAllowed(true)) m_PendingWrite = text;
        m_LocalHash = hash;
        m_Revision++;
    }
    m_LocalKnown = true;
}

bool ClipboardSync::send(QByteArray text)
{
    if (m_Blocked || !m_Focused || !m_Services.clipboardAllowed(true) || !ClipboardText::valid(text)) return false;
    observeLocal(text);
    m_PendingWrite = std::move(text);
    dispatch();
    return true;
}

void ClipboardSync::receive()
{
    if (!m_Blocked && m_Focused && m_Services.clipboardAllowed(false)) {
        m_ReceivePending = true;
        dispatch();
    }
}

void ClipboardSync::dispatch()
{
    if (m_Blocked || !m_Focused || m_ActiveId || m_Clock.elapsed() < m_RetryAt) return;
    const bool send = m_PendingWrite.has_value();
    if (!send && !m_ReceivePending && (!m_Automatic || !m_LocalKnown || m_Clock.elapsed() < m_NextPoll)) return;
    const auto id = m_NextId++;
    if (!m_Services.clipboardRequest(send, send ? *m_PendingWrite : QByteArray(), id)) return;
    m_ActiveId = id;
    m_Sending = send;
    if (send) {
        m_ActiveWrite = std::move(m_PendingWrite);
        m_PendingWrite.reset();
    }
    else {
        m_ReadExplicit = m_ReceivePending;
        m_ReceivePending = false;
        m_NextPoll = m_Clock.elapsed() + 1000;
        m_ReadRevision = m_Revision;
    }
}

std::optional<QByteArray> ClipboardSync::complete(const SessionServiceMessage& message)
{
    if (message.generation != m_Services.generation() || !m_ActiveId || message.requestId != m_ActiveId) return {};
    m_ActiveId = 0;
    m_Feedback = Feedback::None;
    if (message.httpStatus == 401 || message.httpStatus == 403 || message.httpStatus == 404 || message.httpStatus == 405 || message.httpStatus == 410 || message.httpStatus == 501) {
        m_Feedback = message.httpStatus == 401 || message.httpStatus == 403 ? Feedback::Denied : Feedback::Unsupported;
        m_Blocked = true;
        m_PendingWrite.reset();
        m_ActiveWrite.reset();
        m_ReceivePending = false;
        return {};
    }
    if (message.status != SessionServiceMessage::Status::Success) {
        m_Feedback = Feedback::Failed;
        if (m_Sending && m_Focused && !m_PendingWrite) m_PendingWrite = std::move(m_ActiveWrite);
        m_ActiveWrite.reset();
        m_RetryAt = m_Clock.elapsed() + 5000;
        return {};
    }
    if (m_Sending) {
        m_Feedback = Feedback::Sent;
        if (m_ActiveWrite) m_RemoteHash = QCryptographicHash::hash(*m_ActiveWrite, QCryptographicHash::Sha256);
        m_ActiveWrite.reset();
        return {};
    }
    if (!ClipboardText::valid(message.payload)) {
        m_Feedback = Feedback::Invalid;
        m_Blocked = true;
        return {};
    }
    const auto remoteHash = QCryptographicHash::hash(message.payload, QCryptographicHash::Sha256);
    const bool eligible = m_ReadExplicit || (m_RemoteKnown && remoteHash != m_RemoteHash);
    if (eligible && m_Focused && m_ReadRevision == m_Revision && !m_PendingWrite) {
        m_RemoteKnown = true;
        m_RemoteHash = remoteHash;
        observeLocal(message.payload);
        m_Feedback = Feedback::Received;
        return message.payload;
    }
    if (!m_RemoteKnown && m_Focused && m_ReadRevision == m_Revision && !m_PendingWrite) {
        m_RemoteKnown = true;
        m_RemoteHash = remoteHash;
    }
    if (m_ReadExplicit) m_Feedback = Feedback::Skipped;
    return {};
}

ClipboardSync::Feedback ClipboardSync::takeFeedback()
{
    const auto feedback = m_Feedback;
    m_Feedback = Feedback::None;
    return feedback;
}

bool ClipboardSync::blocked() const
{
    return m_Blocked;
}

bool ClipboardSync::receiving() const
{
    return m_ActiveId && !m_Sending;
}

void ClipboardSync::invalidateLocal()
{
    m_Revision++;
}

void ClipboardSync::setFocused(bool focused)
{
    if (focused == m_Focused) return;
    m_Focused = focused;
    invalidateLocal();
    if (!focused) {
        m_PendingWrite.reset();
        m_ReceivePending = false;
    }
}

void ClipboardSync::disableAutomatic()
{
    m_Automatic = false;
    m_PendingWrite.reset();
    invalidateLocal();
}

bool ClipboardSync::automatic() const
{
    return m_Automatic && !m_Blocked;
}
