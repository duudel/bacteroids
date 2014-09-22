
#ifndef H_BACT_PLAYER_H
#define H_BACT_PLAYER_H

#include "GameObject.h"

#include "../Types.h"
#include "../renderer/Renderer.h"
#include "../String.h"

namespace bact
{

    using namespace rob;

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
        Player()
            : GameObject(TYPE)
            , m_direction(0.0f, 1.0f, 0.0f, 0.0f)
            , m_cooldown(0.0f)
        {
            SetRadius(0.8f);
        }

        static void ClampVectorLength(vec4f &v, const float len)
        {
            const float l = v.Length();
            if (l > len)
            {
                v /= l;
                v *= len;
            }
        }

        static vec4f ClampedVectorLength(const vec4f &v, const float len)
        {
            vec4f cv(v);
            ClampVectorLength(cv, len);
            return cv;
        }

        void Update(const GameTime &gameTime, const Input &input, ProjectileArray &projectiles)
        {
            vec4f vel= vec4f::Zero;
            if (input.KeyDown(Keyboard::Scancode::W))
                vel += vec4f::UnitY;
            if (input.KeyDown(Keyboard::Scancode::S))
                vel -= vec4f::UnitY;
            if (input.KeyDown(Keyboard::Scancode::D))
                vel += vec4f::UnitX;
            if (input.KeyDown(Keyboard::Scancode::A))
                vel -= vec4f::UnitX;

            const float dt = gameTime.GetDeltaSeconds();
            const float friction = 0.1f;

            m_velocity += vel;
            m_velocity -= m_velocity * friction;
            ClampVectorLength(m_velocity, 2.0f);
            m_position += m_velocity * dt;

            const float mdx = input.GetMouseDeltaX();
            const float mdy = -input.GetMouseDeltaY();

            const vec4f delta = vec4f(mdx, mdy, 0.0f, 0.0f);
            m_direction += delta * dt;
            ClampVectorLength(m_direction, 2.5f);

            Cooldown(gameTime);
            if (input.ButtonDown(MouseButton::Left))
            {
                Shoot(gameTime, projectiles);
            }
        }

        void Cooldown(const GameTime &gameTime)
        {
            if (m_cooldown > 0.0f)
            {
                m_cooldown -= gameTime.GetDeltaSeconds();
                if (m_cooldown < 0.0f)
                    m_cooldown = 0.0f;
            }
        }

        void Shoot(const GameTime &gameTime, ProjectileArray &projectiles)
        {
            if (m_cooldown <= 0.0f)
            {
                m_cooldown = 0.25f;
                Projectile *p = projectiles.Obtain();
                const vec4f dir = ClampedVectorLength(m_direction, 1.0f);
                p->SetPosition(m_position + dir * m_radius);
                p->SetVelocity(dir.x * 10.0f, dir.y * 10.0f);
            }
        }

        void Render(Renderer *renderer, ShaderProgramHandle fontShader)
        {
            renderer->SetColor(Color(1.0f, 1.0f, 1.6f));
            renderer->DrawFilledCirlce(m_position.x, m_position.y, m_radius, Color(0.2f, 0.5f, 0.5f, 0.5f));

            renderer->SetColor(Color(2.0f, 1.0f, 0.6f));
            const vec4f dpos = m_position + ClampedVectorLength(m_direction, 0.5f);
            renderer->BindColorShader();
            renderer->DrawFilledCirlce(dpos.x, dpos.y, m_radius * 0.5f, Color(0.2f, 0.5f, 0.5f, 0.5f));

//            const vec4f dpos2 = m_position + m_direction;
//            renderer->DrawFilledCirlce(dpos2.x, dpos2.y, m_radius * 0.5f, Color(0.2f, 0.5f, 0.5f, 0.5f));

//            char velStr[32];
//            StringPrintF(velStr, "%.3f,%.3f", m_velocity.x, m_velocity.y);

//            renderer->SetColor(Color::Red);
//            renderer->BindShader(fontShader);
//            renderer->DrawText(m_position.x, m_position.y, velStr);
        }

    private:
        vec4f m_direction;
        float m_cooldown;
    };

} // bact

#endif // H_BACT_PLAYER_H

