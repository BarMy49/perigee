#include "overlaymanager.h"
#include "path.h"

using namespace Overlay;

OverlayManager::OverlayManager() :
    m_Renderer(nullptr),
    m_FontData(Path::readDataFile("ModeSeven.ttf"))
{
    memset(m_Overlays, 0, sizeof(m_Overlays));

    m_Overlays[OverlayType::OverlayDebug].color = {0xD0, 0xD0, 0x00, 0xFF};
    m_Overlays[OverlayType::OverlayDebug].fontSize = 20;

    m_Overlays[OverlayType::OverlayStatusUpdate].color = {0xCC, 0x00, 0x00, 0xFF};
    m_Overlays[OverlayType::OverlayStatusUpdate].fontSize = 36;

    // While TTF will usually not be initialized here, it is valid for that not to
    // be the case, since Session destruction is deferred and could overlap with
    // the lifetime of a new Session object.
    //SDL_assert(TTF_WasInit() == 0);

    if (TTF_Init() != 0) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                    "TTF_Init() failed: %s",
                    TTF_GetError());
        return;
    }
}

OverlayManager::~OverlayManager()
{
    for (int i = 0; i < OverlayType::OverlayMax; i++) {
        if (m_Overlays[i].surface != nullptr) {
            SDL_FreeSurface(m_Overlays[i].surface);
        }
        if (m_Overlays[i].font != nullptr) {
            TTF_CloseFont(m_Overlays[i].font);
        }
    }

    if (m_NotificationFont != nullptr) TTF_CloseFont(m_NotificationFont);
    TTF_Quit();

    // For similar reasons to the comment in the constructor, this will usually,
    // but not always, deinitialize TTF. In the cases where Session objects overlap
    // in lifetime, there may be an additional reference on TTF for the new Session
    // that means it will not be cleaned up here.
    //SDL_assert(TTF_WasInit() == 0);
}

bool OverlayManager::isOverlayEnabled(OverlayType type)
{
    return m_Overlays[type].enabled || (type == OverlayStatusUpdate && m_NotificationActive.load());
}

char* OverlayManager::getOverlayText(OverlayType type)
{
    std::lock_guard<std::recursive_mutex> lock(m_NotificationMutex);
    if (type == OverlayStatusUpdate && !m_Overlays[type].enabled && m_NotificationActive.load()) return m_NotificationText;
    return m_Overlays[type].text;
}

void OverlayManager::updateOverlayText(OverlayType type, const char* text)
{
    SDL_utf8strlcpy(m_Overlays[type].text, text, sizeof(m_Overlays[0].text));
    setOverlayTextUpdated(type);
}

int OverlayManager::getOverlayMaxTextLength()
{
    return sizeof(m_Overlays[0].text);
}

int OverlayManager::getOverlayFontSize(OverlayType type)
{
    std::lock_guard<std::recursive_mutex> lock(m_NotificationMutex);
    if (type == OverlayStatusUpdate && !m_Overlays[type].enabled && m_NotificationActive.load()) return 24;
    return m_Overlays[type].fontSize;
}

SDL_Surface* OverlayManager::getUpdatedOverlaySurface(OverlayType type)
{
    // If a new surface is available, return it. If not, return nullptr.
    // Caller must free the surface on success.
    return (SDL_Surface*)SDL_AtomicSetPtr((void**)&m_Overlays[type].surface, nullptr);
}

void OverlayManager::setOverlayTextUpdated(OverlayType type)
{
    // Only update the overlay state if it's enabled. If it's not enabled,
    // the renderer has already been notified by setOverlayState().
    if (m_Overlays[type].enabled) {
        notifyOverlayUpdated(type);
    }
}

void OverlayManager::setOverlayState(OverlayType type, bool enabled)
{
    bool stateChanged = m_Overlays[type].enabled != enabled;

    m_Overlays[type].enabled = enabled;

    if (stateChanged) {
        if (!enabled) {
            // Set the text to empty string on disable
            m_Overlays[type].text[0] = 0;
        }

        notifyOverlayUpdated(type);
    }
}

SDL_Color OverlayManager::getOverlayColor(OverlayType type)
{
    std::lock_guard<std::recursive_mutex> lock(m_NotificationMutex);
    if (type == OverlayStatusUpdate && !m_Overlays[type].enabled && m_NotificationActive.load()) return {0xEE, 0xEE, 0xEE, 0xFF};
    return m_Overlays[type].color;
}

void OverlayManager::setOverlayRenderer(IOverlayRenderer* renderer)
{
    m_Renderer = renderer;
}

void OverlayManager::notifyOverlayUpdated(OverlayType type)
{
    if (m_Renderer == nullptr) {
        return;
    }

    SDL_Surface* oldSurface;
    {
        std::lock_guard<std::recursive_mutex> lock(m_NotificationMutex);
        SDL_Surface* surface = nullptr;
        if (isOverlayEnabled(type)) {
            const bool notification = type == OverlayStatusUpdate && !m_Overlays[type].enabled;
            TTF_Font*& font = notification ? m_NotificationFont : m_Overlays[type].font;
            if (font == nullptr) {
                if (m_FontData.isEmpty()) {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL overlay font failed to load");
                    return;
                }
                font = TTF_OpenFontRW(SDL_RWFromConstMem(m_FontData.constData(), m_FontData.size()),
                                      1, getOverlayFontSize(type));
                if (font == nullptr) {
                    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unable to load overlay font");
                    return;
                }
            }
            surface = RenderTextOutlinedWrapped(font, getOverlayText(type), getOverlayColor(type), {0, 0, 0, 255}, 4, 1024);
        }
        oldSurface = (SDL_Surface*)SDL_AtomicSetPtr((void**)&m_Overlays[type].surface, surface);
    }
    m_Renderer->notifyOverlayUpdated(type);
    SDL_FreeSurface(oldSurface);
}

void OverlayManager::showNotification(const QString& text, Uint32 now)
{
    bool changed;
    {
        std::lock_guard<std::recursive_mutex> lock(m_NotificationMutex);
        const auto utf8 = text.toUtf8();
        char nextText[sizeof(m_NotificationText)];
        SDL_utf8strlcpy(nextText, utf8.constData(), sizeof(nextText));
        changed = !m_NotificationActive.load() || SDL_strcmp(m_NotificationText, nextText) != 0;
        SDL_utf8strlcpy(m_NotificationText, nextText, sizeof(m_NotificationText));
        m_NotificationDeadline = now + 5000;
        m_NotificationActive.store(true);
    }
    if (changed && !m_Overlays[OverlayStatusUpdate].enabled) notifyOverlayUpdated(OverlayStatusUpdate);
}

void OverlayManager::expireNotification(Uint32 now)
{
    if (!m_NotificationActive.load()) return;
    bool expired = false;
    {
        std::lock_guard<std::recursive_mutex> lock(m_NotificationMutex);
        if (m_NotificationActive.load() && SDL_TICKS_PASSED(now, m_NotificationDeadline)) {
            m_NotificationActive.store(false);
            expired = true;
        }
    }
    if (expired && !m_Overlays[OverlayStatusUpdate].enabled) notifyOverlayUpdated(OverlayStatusUpdate);
}

int OverlayManager::notificationTimeout(int maximum, Uint32 now)
{
    if (!m_NotificationActive.load()) return maximum;
    std::lock_guard<std::recursive_mutex> lock(m_NotificationMutex);
    if (SDL_TICKS_PASSED(now, m_NotificationDeadline)) return 0;
    return qMin(maximum, int(m_NotificationDeadline - now));
}


SDL_Surface* OverlayManager::RenderTextOutlinedWrapped(TTF_Font* font, const char* text, SDL_Color textColor, SDL_Color outlineColor, int outlineWidth, int wrapWidth) {
    if (text == nullptr || text[0] == '\0') {
        return nullptr;
    }

    int oldOutline = TTF_GetFontOutline(font);
    TTF_SetFontOutline(font, outlineWidth);

    // Verify that the string won't require wrapping (which could cause the outline and the text
    // to diverge due to different wrapping positions).
    //
    // FIXME: We do this rather than just disabling wrapping entirely (wrapWidth = 0) because we
    // need further testing to ensure that all renderers can handle non-NPOT overlay textures.
    for (const QString& line : QString(text).split('\n')) {
        int extent, count;
        if (TTF_MeasureUTF8(font, line.toUtf8(), wrapWidth, &extent, &count) == 0 && count < line.size()) {
            // If it requires wrapping, render it without the outline
            TTF_SetFontOutline(font, oldOutline);
            return TTF_RenderUTF8_Blended_Wrapped(font, text, textColor, wrapWidth);
        }
    }

    // Draw text twice, but outline is a bit bigger
    auto outlineSurface = TTF_RenderUTF8_Blended_Wrapped(font, text, outlineColor, wrapWidth);
    TTF_SetFontOutline(font, 0);
    auto textSurface = TTF_RenderUTF8_Blended_Wrapped(font, text, textColor, wrapWidth);
    TTF_SetFontOutline(font, oldOutline);

    if (outlineSurface == nullptr || textSurface == nullptr) {
        SDL_FreeSurface(outlineSurface);
        SDL_FreeSurface(textSurface);
        return nullptr;
    }

    // Merge the texts
    SDL_Rect dst = { outlineWidth, outlineWidth, textSurface->w, textSurface->h };
    SDL_BlitSurface(textSurface, nullptr, outlineSurface, &dst);

    SDL_FreeSurface(textSurface);
    return outlineSurface;
}


