#pragma once

#include <QByteArray>
#include <SDL.h>
#include <optional>

class SdlClipboard
{
public:
    static constexpr qint64 AutomaticReadBudgetMs = 4;
    static bool automaticSupported(const QByteArray& videoDriver, const QByteArray& desktop);
    static std::optional<QByteArray> read(SDL_Window* window, qint64* elapsedMs = nullptr);
    static bool write(SDL_Window* window, const QByteArray& text);
};
