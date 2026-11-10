#pragma once

#include <SDL_events.h>
#include <bitset>

namespace LocalShortcut
{
class ConsumedKeys
{
public:
    bool filter(const SDL_KeyboardEvent& event)
    {
        const int scan = event.keysym.scancode;
        if (scan <= SDL_SCANCODE_UNKNOWN || scan >= SDL_NUM_SCANCODES) return false;
        if (event.state == SDL_RELEASED) {
            const bool consumed = m_Keys.test(scan);
            m_Keys.reset(scan);
            return consumed;
        }
        if (!event.repeat) m_Keys.reset(scan);
        return event.repeat && m_Keys.test(scan);
    }

    void consume(SDL_Scancode scan)
    {
        if (scan > SDL_SCANCODE_UNKNOWN && scan < SDL_NUM_SCANCODES) m_Keys.set(scan);
    }

private:
    std::bitset<SDL_NUM_SCANCODES> m_Keys;
};

inline bool modifiersMatch(Uint16 modifiers)
{
    return (modifiers & KMOD_CTRL) &&
           (modifiers & KMOD_ALT) &&
           (modifiers & KMOD_SHIFT) &&
           !(modifiers & KMOD_MODE);
}
}
