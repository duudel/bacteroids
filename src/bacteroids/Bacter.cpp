
#include "Bacter.h"
#include "ObjectArray.h"

#include "../renderer/Renderer.h"
#include "../graphics/Graphics.h"

#include "../math/Random.h"

namespace bact
{

    Bacter::Bacter()
        : GameObject(TYPE)
        , m_anim(0.0f)
        , m_size(BacterMinSize), m_sizeMod(1.0f)
        , m_target(nullptr)
        , m_points(10)
        , m_splitTimer(0.0f)
        , m_readyToSplitTimer(0.0f)
    {
        SetRadius(1.0f);
    }

    void Bacter::Setup(const GameObject *target, Random &random)
    {
        SetTarget(target);
        m_anim = random.GetReal(0.0, 2.0 * PI_d);
    }

//        int TakeHit(BacterArray &bacterArray, Random &random)
    int Bacter::TakeHit(ObjectArray &bacterArray, Random &random)
    {
        if (m_splitTimer > 0.0f) return 0;

        int points = m_points;

        if (m_size / 2.0f < BacterMinSize)
        {
            m_alive = false;
        }
        else if (!Bacter::Split(this, bacterArray, random))
        {
            SplitSelf();
        }

        return points;
    }

    void Bacter::ModifySize(float sizeMod)
    {
        m_sizeMod = Max(m_sizeMod, sizeMod);
    }

    void Bacter::Update(const GameTime &gameTime)
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

    void Bacter::SplitSelf()
    {
        m_size = m_size / 2.0f;
        m_points = m_points / 2.0f + 0.6f; // points is at least 1
        m_splitTimer = 0.1f;
        m_readyToSplitTimer = 0.0f;
    }

//        static void TrySplit(Bacter *bacter, BacterArray &bacterArray, Random &random)
    void Bacter::TrySplit(Bacter *bacter, ObjectArray &bacterArray, Random &random)
    {
        if (bacter->m_readyToSplitTimer >= 6.0f)
            Split(bacter, bacterArray, random);
    }

//        static bool Split(Bacter *bacter, BacterArray &bacterArray, Random &random)
    bool Bacter::Split(Bacter *bacter, ObjectArray &bacterArray, Random &random)
    {
        if (!bacterArray.CanObtainBacter()) return false;

        Bacter *bacter2 = bacterArray.ObtainBacter();
        bacter2->Setup(bacter->m_target, random);
        bacter2->SetPosition(bacter->GetPosition() + random.GetDirection()*0.1f);
        bacter2->SetRadius(bacter->GetRadius());
        bacter2->m_size = bacter->m_size;
        bacter2->m_points = bacter->m_points;
        bacter->SplitSelf();
        bacter2->SplitSelf();

        return true;
    }

    void Bacter::Render(Renderer *renderer, const BacteroidsUniforms &uniforms)
    {
        renderer->GetGraphics()->SetUniform(uniforms.m_anim, m_anim);
        renderer->SetColor(Color(1.0f, 1.2f, 0.6f));
//            renderer->DrawFilledCirlce(m_position.x, m_position.y, m_radius);
        renderer->DrawFilledCirlce(m_position.x, m_position.y, m_radius, Color(0.0f, 0.5f, 0.5f, 1.0f));
    }

} // bact
