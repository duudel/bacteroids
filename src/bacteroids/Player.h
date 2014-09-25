
#ifndef H_BACT_PLAYER_H
#define H_BACT_PLAYER_H

#include "GameObject.h"

#include "../input/Keyboard.h"
#include "../input/Mouse.h"

namespace bact
{

    using namespace rob;

    class ObjectArray;
    class SoundPlayer;

    class Input
    {
    public:
        void SetKey(Keyboard::Scancode key, bool down)
        {
            m_keys[static_cast<size_t>(key)] = down;
        }

        bool KeyDown(Keyboard::Scancode key) const
        {
            return m_keys[static_cast<size_t>(key)];
        }

        void SetButtons(bool left, bool right, bool mid)
        {
            m_buttons[static_cast<size_t>(MouseButton::Left)] = left;
            m_buttons[static_cast<size_t>(MouseButton::Right)] = right;
            m_buttons[static_cast<size_t>(MouseButton::Middle)] = mid;
        }

        bool ButtonDown(MouseButton button) const
        {
            return m_buttons[static_cast<size_t>(button)];
        }

        void SetMouse(float mx, float my, float dx, float dy)
        {
            m_mousePositionX = mx;
            m_mousePositionY = my;
            m_mouseDeltaX = dx;
            m_mouseDeltaY = dy;
        }

        float GetMouseX() const { return m_mousePositionX; }
        float GetMouseY() const { return m_mousePositionY; }
        float GetMouseDeltaX() const { return m_mouseDeltaX; }
        float GetMouseDeltaY() const { return m_mouseDeltaY; }

    private:
        bool m_keys[static_cast<size_t>(Keyboard::Scancode::NUM_KEYS)];
        bool m_buttons[static_cast<size_t>(MouseButton::NUM_BUTTONS)];
        float m_mousePositionX;
        float m_mousePositionY;
        float m_mouseDeltaX;
        float m_mouseDeltaY;
    };

    class Player : public GameObject
    {
    public:
        static const int TYPE = 1;
    public:
        Player();

        static void ClampVectorLength(vec2f &v, const float len);
        static vec2f ClampedVectorLength(const vec2f &v, const float len);

        void Update(const GameTime &gameTime, const Input &input, ObjectArray &projectiles, SoundPlayer &sounds);

        void Cooldown(const GameTime &gameTime);
        void Shoot(const GameTime &gameTime, ObjectArray &projectiles, SoundPlayer &sounds);

        void Render(Renderer *renderer, const BacteroidsUniforms &uniforms) override;

    private:
        vec2f m_direction;
        float m_cooldown;
    };

} // bact

#endif // H_BACT_PLAYER_H

