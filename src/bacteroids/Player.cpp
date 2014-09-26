
#include "Player.h"
#include "ObjectArray.h"
#include "SoundPlayer.h"
#include "Input.h"

#include "../renderer/Renderer.h"
#include "../graphics/Graphics.h"

#include "../application/GameTime.h"


namespace bact
{

    Player::Player()
        : GameObject(TYPE)
        , m_direction(0.0f, 1.0f)
        , m_health(100.0f)
        , m_cooldown(0.0f)
    {
        SetRadius(0.8f);
    }

    void Player::TakeHit()
    {
        m_health -= 0.2f;
        if (m_health <= 0.0f)
            m_alive = false;
    }

    float Player::GetHealth() const
    { return m_health; }

    void Player::Update(const GameTime &gameTime, const Input &input, ObjectArray &projectiles, SoundPlayer &sounds)
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

        const vec2f delta = input.GetMouseDelta() * vec2f(1.0f, -1.0f);
        m_direction += delta * dt;
        ClampVectorLength(m_direction, 2.5f);

        Cooldown(gameTime);
        if (input.ButtonDown(MouseButton::Left))
        {
            Shoot(gameTime, projectiles, sounds);
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

    void Player::Shoot(const GameTime &gameTime, ObjectArray &projectiles, SoundPlayer &sounds)
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

            sounds.PlayShootSound();
        }
    }

    void Player::Render(Renderer *renderer, const BacteroidsUniforms &uniforms)
    {
        renderer->SetColor(Color(1.0f, 1.0f, 1.6f));
        renderer->DrawFilledCirlce(m_position.x, m_position.y, m_radius, Color(0.2f, 0.5f, 0.5f, 0.5f));

        renderer->SetColor(Color(2.0f, 1.0f, 0.6f));
        const vec2f dpos = m_position + ClampedVectorLength(m_direction, 1.5f);
        renderer->BindColorShader();
        renderer->DrawFilledCirlce(dpos.x, dpos.y, m_radius * 0.5f, Color(0.2f, 0.5f, 0.5f, 0.5f));
    }

} // bact
