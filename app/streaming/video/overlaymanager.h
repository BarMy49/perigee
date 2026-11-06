#pragma once

#include <QString>
#include <QStringList>
#include <QHash>
#include <atomic>
#include <mutex>

#include "SDL_compat.h"
#include <SDL_ttf.h>

namespace Overlay {

enum OverlayType {
    OverlayDebug,
    OverlayStatusUpdate,
    OverlayMax
};

class IOverlayRenderer
{
public:
    virtual ~IOverlayRenderer() = default;

    virtual void notifyOverlayUpdated(OverlayType type) = 0;
};

class OverlayManager
{
public:
    OverlayManager();
    ~OverlayManager();

    bool isOverlayEnabled(OverlayType type);
    char* getOverlayText(OverlayType type);
    void updateOverlayText(OverlayType type, const char* text);
    int getOverlayMaxTextLength();
    void setOverlayTextUpdated(OverlayType type);
    void setOverlayState(OverlayType type, bool enabled);
    SDL_Color getOverlayColor(OverlayType type);
    int getOverlayFontSize(OverlayType type);
    SDL_Surface* getUpdatedOverlaySurface(OverlayType type);

    void setOverlayRenderer(IOverlayRenderer* renderer);
    void prepareCommandPanel(const QString& text);
    bool setCommandPanelVisible(bool visible);
    bool isCommandPanelVisible() const;
    void prepareNotifications(const QStringList& texts);
    void showNotification(const QString& text, Uint32 now = SDL_GetTicks());
    void expireNotification(Uint32 now = SDL_GetTicks());
    int notificationTimeout(int maximum, Uint32 now = SDL_GetTicks());

private:
    void notifyOverlayUpdated(OverlayType type);
    SDL_Surface* RenderTextOutlinedWrapped(TTF_Font* font, const char* text, SDL_Color textColor, SDL_Color outlineColor, int outlineWidth, int wrapWidth);

    struct {
        bool enabled;
        int fontSize;
        SDL_Color color;
        char text[1024];

        TTF_Font* font;
        SDL_Surface* surface;
    } m_Overlays[OverlayMax];
    IOverlayRenderer* m_Renderer;
    QByteArray m_FontData;
    std::recursive_mutex m_NotificationMutex;
    std::atomic<bool> m_NotificationActive{false};
    std::atomic<bool> m_CommandPanelActive{false};
    char m_CommandPanelText[1024] = {};
    SDL_Surface* m_CommandPanelSurface = nullptr;
    Uint32 m_NotificationDeadline = 0;
    char m_NotificationText[1024] = {};
    QHash<QString, SDL_Surface*> m_NotificationSurfaces;
};

}
