
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
        { m_keys[static_cast<size_t>(key)] = down; }

        bool KeyDown(Keyboard::Scancode key) const
        { return m_keys[static_cast<size_t>(key)]; }

        void UpdateMouse()
        { GetMouseState(m_mouse); }

        bool ButtonDown(MouseButton button) const
        { return m_mouse.ButtonDown(button); }

        vec2f GetMousePosition() const
        { return m_mouse.GetPosition(); }

        vec2f GetMouseDelta() const
        { return m_mouse.GetDeltaPosition(); }

    private:
        bool m_keys[static_cast<size_t>(Keyboard::Scancode::NUM_KEYS)];
        Mouse m_mouse;
    };

    class Player : public GameObject
    {
    public:
        static const int TYPE = 1;
    public:
        Player();

        void Update(const GameTime &gameTime, const Input &input, ObjectArray &projectiles, SoundPlayer &sounds);

        void Cooldown(const GameTime &gameTime);
        void Shoot(const GameTime &gameTime, ObjectArray &projectiles, SoundPlayer &sounds);

        void Render(Renderer *renderer, const BacteroidsUniforms &uniforms) override;

    private:
        vec2f m_direction;
        float m_health;
        float m_cooldown;
    };

} // bact

#endif // H_BACT_PLAYER_H

