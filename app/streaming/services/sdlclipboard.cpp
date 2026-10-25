#include "sdlclipboard.h"
#include "clipboardtext.h"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QThread>
#include <cstring>

bool SdlClipboard::automaticSupported(const QByteArray& videoDriver, const QByteArray& desktop)
{
#ifdef Q_OS_LINUX
    return videoDriver == "wayland" && desktop.toUpper().split(':').contains("GNOME");
#else
    Q_UNUSED(videoDriver);
    Q_UNUSED(desktop);
    return false;
#endif
}

std::optional<QByteArray> SdlClipboard::read(SDL_Window* window, qint64* elapsedMs)
{
    Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());
    if (!window || !(SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS)) return {};
    QElapsedTimer timer;
    timer.start();
    SDL_ClearError();
    char* text = SDL_GetClipboardText();
    if (elapsedMs) *elapsedMs = timer.elapsed();
    else SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Manual clipboard read took %lld ms", static_cast<long long>(timer.elapsed()));
    if (!text || *SDL_GetError()) {
        SDL_free(text);
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Clipboard text is unavailable on this window/backend");
        return {};
    }
    const size_t length = strnlen(text, 1024 * 1024 + 1);
    std::optional<QByteArray> result;
    if (length <= 1024 * 1024) {
        QByteArray value(text, int(length));
        if (ClipboardText::valid(value)) result = std::move(value);
    }
    SDL_free(text);
    return result;
}

bool SdlClipboard::write(SDL_Window* window, const QByteArray& text)
{
    Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());
    if (!window || !(SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS) || !ClipboardText::valid(text)) return false;
    const bool success = SDL_SetClipboardText(text.constData()) == 0;
    if (!success) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unable to apply received clipboard text");
    return success;
}
