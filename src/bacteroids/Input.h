
#ifndef H_BACT_INPUT_H
#define H_BACT_INPUT_H

#include "../input/Keyboard.h"
#include "../input/Mouse.h"

namespace bact
{

    using namespace rob;

    class Input
    {
    public:
        void SetKey(Keyboard::Scancode key, bool down)
        { m_keys[static_cast<size_t_32>(key)] = down; }

        bool KeyDown(Keyboard::Scancode key) const
        { return m_keys[static_cast<size_t_32>(key)]; }

        void UpdateMouse()
        { GetMouseState(m_mouse); }

        bool ButtonDown(MouseButton button) const
        { return m_mouse.ButtonDown(button); }

        vec2f GetMousePosition() const
        { return m_mouse.GetPosition(); }

        vec2f GetMouseDelta() const
        { return m_mouse.GetDeltaPosition(); }

    private:
        bool m_keys[static_cast<size_t_32>(Keyboard::Scancode::NUM_KEYS)];
        Mouse m_mouse;
    };

} // bact

#endif // H_BACT_INPUT_H

