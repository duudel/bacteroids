
#ifndef H_BACT_PLAYER_H
#define H_BACT_PLAYER_H

#include "../Types.h"
#include "../math/Math.h"
#include "../renderer/Renderer.h"
#include "../application/GameTime.h"
#include "../String.h"

namespace bact
{

    using namespace rob;

    class Input
    {
    public:
        bool KeyDown(Key key) const
        {
            uint32_t k = static_cast<uint32_t>(key);
            return m_keys[k];
        }

        void SetKey(Key key, bool down)
        {
            uint32_t k = static_cast<uint32_t>(key);
            m_keys[k] = down;
        }
    private:
        bool m_keys[256];
    };

    class Player
    {
    public:
        Player()
            : m_position(0.0f, 0.0f, 0.0f, 1.0f)
            , m_velocity(0.0f, 0.0f, 0.0f, 0.0f)
            , m_radius(0.8f)
        { }

        void SetPosition(float x, float y)
        { m_position = vec4f(x, y, 0.0f, 1.0f); }
        void SetPosition(const vec4f &p)
        { m_position = vec4f(p.x, p.y, 0.0f, 1.0f); }
        void SetRadius(float r)
        { m_radius = r; }
        void SetVelocity(float x, float y)
        { m_velocity = vec4f(x, y, 0.0f, 0.0f); }

        vec4f GetPosition() const
        { return m_position; }
        float GetRadius() const
        { return m_radius; }

        void AddVelocity(const vec4f &v)
        { m_velocity += v; }

        static void ClampVectorLength(vec4f &v, const float len)
        {
            const float l = v.Length();
            if (l > len)
            {
                v /= l;
                v *= len;
            }
        }

        void Update(const GameTime &gameTime, const Input &input)
        {
            vec4f vel= vec4f::Zero;
            if (input.KeyDown(Key::W))
                vel += vec4f::UnitY;
            if (input.KeyDown(Key::S))
                vel -= vec4f::UnitY;
            if (input.KeyDown(Key::D))
                vel += vec4f::UnitX;
            if (input.KeyDown(Key::A))
                vel -= vec4f::UnitX;

            const float dt = gameTime.GetDeltaSeconds();
            const float friction = 0.1f;

            m_velocity += vel;
            m_velocity -= m_velocity * friction;
            ClampVectorLength(m_velocity, 2.0f);
            m_position += m_velocity * dt;
        }

        void Render(Renderer *renderer, ShaderProgramHandle fontShader)
        {
            renderer->SetColor(Color(1.0f, 1.0f, 1.6f));
            renderer->DrawFilledCirlce(m_position.x, m_position.y, m_radius, Color(0.2f, 0.5f, 0.5f, 0.5f));

            char velStr[32];
            StringPrintF(velStr, "%.3f,%.3f", m_velocity.x, m_velocity.y);

            renderer->SetColor(Color::Red);
            renderer->BindShader(fontShader);
            renderer->DrawText(m_position.x, m_position.y, velStr);
        }

    private:
        vec4f m_position;
        vec4f m_velocity;
        float m_radius;
    };

} // bact

#endif // H_BACT_PLAYER_H

