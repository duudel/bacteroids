
#include "Bacter.h"
//#include "SoundPlayer.h"

#include "../renderer/Renderer.h"
#include "../graphics/Graphics.h"

#include "../application/GameTime.h"

#include "../math/Random.h"

namespace bact
{

    BACT_GAME_OBJECT_SHADER(Bacter) = InvalidHandle;

    const float MIN_BACTER_SIZE = 0.3f;

    Bacter::Bacter()
        : m_anim(0.0f)
        , m_size(MIN_BACTER_SIZE), m_sizeMod(1.0f)
        , m_target(nullptr)
        , m_points(10)
        , m_splitTimer(0.0f)
        , m_readyToSplitTimer(0.0f)
    {
        SetRadius(1.0f);
    }

    void Bacter::RandomizeAnimation(Random &random)
    {
        m_anim = random.GetReal(0.0, 2.0 * PI_d);
    }

    bool Bacter::WantsToSplit() const
    {
        return (m_readyToSplitTimer >= 6.0f);
    }

    bool Bacter::CanSplit() const
    {
        return (m_splitTimer <= 0.0f);
    }

    bool Bacter::DiesIfSplits() const
    {
        return (m_size / 2.0f < MIN_BACTER_SIZE);
    }

    void Bacter::Split(Random &random)
    {
        if (m_size / 2.0f < MIN_BACTER_SIZE)
        {
            m_alive = false;
            return;
        }
        m_size = m_size / 2.0f;
        m_points = m_points / 2.0f + 0.6f; // points is at least 1
        m_splitTimer = 0.1f;
        m_readyToSplitTimer = 0.0f;
        RandomizeAnimation(random);
        m_position += random.GetDirection() * 0.1f;
        return;
    }

    void Bacter::Update(const GameTime &gameTime, const Rect &playArea)
    {
        const float dt = gameTime.GetDeltaSeconds();

        if (m_target)
        {
            vec2f a = 0.4f * (m_target->GetPosition() - m_position).SafeNormalized();
            vec2f v = m_velocity + a*dt;
            if (v.Length() > 1.0f && m_velocity.Length() < v.Length()) ;
            else m_velocity = v;
        }

        if (m_splitTimer > 0.0f)
        {
            m_velocity *= 0.6f;
            m_splitTimer -= dt;
        }
        else
        {
            if (m_size < 1.0f)
                m_size += 0.05f * dt;
            else
            {
                m_readyToSplitTimer += dt;
            }
        }

        m_position += m_velocity * dt;
        m_velocity -= m_velocity * 0.2f * dt;

        m_radius = Sqrt(m_size * m_sizeMod);
        m_sizeMod = 1.0f;
    }

    void Bacter::Render(Renderer *renderer, const BacteroidsUniforms &uniforms)
    {
        renderer->GetGraphics()->SetUniform(uniforms.m_anim, m_anim);
        renderer->SetColor(Color(1.0f, 1.2f, 0.6f));
//            renderer->DrawFilledCirlce(m_position.x, m_position.y, m_radius);
        renderer->DrawFilledCirlce(m_position.x, m_position.y, m_radius, Color(0.0f, 0.5f, 0.5f, 1.0f));
    }

} // bact
