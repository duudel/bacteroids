
#include "Player.h"
#include "ObjectArray.h"

#include "../renderer/Renderer.h"
#include "../graphics/Graphics.h"

#include "../application/GameTime.h"


namespace bact
{

    Player::Player()
        : GameObject(TYPE)
        , m_direction(0.0f, 1.0f)
        , m_cooldown(0.0f)
    {
        SetRadius(0.8f);
    }

    //static
    void Player::ClampVectorLength(vec2f &v, const float len)
    {
        const float l = v.Length();
        if (l > len)
        {
            v /= l;
            v *= len;
        }
    }

    //static
    vec2f Player::ClampedVectorLength(const vec2f &v, const float len)
    {
        vec2f cv(v);
        ClampVectorLength(cv, len);
        return cv;
    }

    void Player::Update(const GameTime &gameTime, const Input &input, ObjectArray &projectiles, AudioSystem &audio)
    {
        vec2f vel= vec2f::Zero;
        if (input.KeyDown(Keyboard::Scancode::W))
            vel += vec2f::UnitY;
        if (input.KeyDown(Keyboard::Scancode::S))
            vel -= vec2f::UnitY;
        if (input.KeyDown(Keyboard::Scancode::D))
            vel += vec2f::UnitX;
        if (input.KeyDown(Keyboard::Scancode::A))
            vel -= vec2f::UnitX;

        const float dt = gameTime.GetDeltaSeconds();
        const float friction = 0.1f;

        m_velocity += vel;
        m_velocity -= m_velocity * friction;
        ClampVectorLength(m_velocity, 2.0f);
        m_position += m_velocity * dt;

        const float mdx = input.GetMouseDeltaX();
        const float mdy = -input.GetMouseDeltaY();

        const vec2f delta(mdx, mdy);
        m_direction += delta * dt;
        ClampVectorLength(m_direction, 2.5f);

        Cooldown(gameTime);
        if (input.ButtonDown(MouseButton::Left))
        {
            Shoot(gameTime, projectiles, audio);
        }
    }

    void Player::Cooldown(const GameTime &gameTime)
    {
        if (m_cooldown > 0.0f)
        {
            m_cooldown -= gameTime.GetDeltaSeconds();
            if (m_cooldown < 0.0f)
                m_cooldown = 0.0f;
        }
    }

    void Player::Shoot(const GameTime &gameTime, ObjectArray &projectiles, AudioSystem &audio)
    {
        if (m_cooldown <= 0.0f)
        {
            m_cooldown = 0.1f;
            if (!projectiles.CanObtainProjectile()) return;

            Projectile *p = projectiles.ObtainProjectile();
//            const vec2f dir = ClampedVectorLength(m_direction, 1.0f);
            vec2f dir = m_direction.SafeNormalized();
            if (dir.Length() < 0.9f) dir = vec2f::UnitX;
            p->SetPosition(m_position + dir * m_radius);
            p->SetVelocity(dir.x * 10.0f, dir.y * 10.0f);

            audio.PlaySound(shootSound, 0.5f);
        }
    }

    void Player::Render(Renderer *renderer, const BacteroidsUniforms &uniforms) //, ShaderProgramHandle fontShader)
    {
        vec4f velocity(m_velocity.x, m_velocity.y, 0.0f, 0.0f);
        renderer->GetGraphics()->SetUniform(uniforms.m_velocity, velocity);
        renderer->SetColor(Color(1.0f, 1.0f, 1.6f));
        renderer->DrawFilledCirlce(m_position.x, m_position.y, m_radius, Color(0.2f, 0.5f, 0.5f, 0.5f));

        renderer->SetColor(Color(2.0f, 1.0f, 0.6f));
        const vec2f dpos = m_position + ClampedVectorLength(m_direction, 1.5f);
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

} // bact
