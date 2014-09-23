
#ifndef H_BACT_BACTER_H
#define H_BACT_BACTER_H

#include "GameObject.h"
#include "Player.h"
#include "BacterArray.h"

#include "../renderer/Renderer.h"
#include "../graphics/Graphics.h"

#include "../math/Math.h"
#include "../math/Vector2.h"
#include "../math/Random.h"

namespace bact
{

    class Player;

    using namespace rob;

    class Bacter : public GameObject
    {
    public:
        static const int TYPE = 2;
    public:
        Bacter()
            : GameObject(TYPE)
            , m_anim(0.0f)
            , m_size(0.5f), m_sizeMod(1.0f)
            , m_target(nullptr)
            , m_health(10)
        {
            SetRadius(1.0f);
        }

        void Setup(const GameObject *target, Random &random)
        {
            SetTarget(target);
            m_anim = random.GetReal(0.0, 2.0 * PI_d);
        }

        void SetTarget(const GameObject *target)
        { m_target = target; }

        void TakeHit()
        {
            m_health -= 10;
            if (m_health <= 0)
                m_alive = false;
        }

        void DoCollision(Bacter *b)
        {
            vec2f A  = GetPosition();
            float Ar = GetRadius();
            vec2f B  = b->GetPosition();
            float Br = b->GetRadius();

            vec2f BA = A - B;
            float r = Ar + Br;
            float d = r - BA.Length();
            if (d > 0.0f)
            {
//                if (d > 0.3f)
                {
                    vec2f v = BA.SafeNormalized() * d/8.0f;
                    AddVelocity(v);
                    b->AddVelocity(-v);
                    SetPosition(A + v/2.0f);
                    b->SetPosition(B - v/2.0f);
                }
//                else
//                {
//                    vec2f v = Normalize(BA) * d/8.0f;
//                    SetPosition(A + v);
//                    b->SetPosition(B - v);
//                }
                m_sizeMod = Max(m_sizeMod, 1.0f + d / r);
                b->m_sizeMod = Max(b->m_sizeMod, m_sizeMod);
            }
        }

        void DoCollision(Player *pl)
        {
            vec2f A  = GetPosition();
            float Ar = GetRadius();
            vec2f B  = pl->GetPosition();
            float Br = pl->GetRadius();

            vec2f BA = A - B;
            float r = Ar + Br;
            float d = r - BA.Length();
            if (d > 0.0f)
            {
                vec2f v = BA.SafeNormalized() * d/8.0f;
                AddVelocity(v);
                pl->AddVelocity(-v);
                SetPosition(A + v/2.0f);
                pl->SetPosition(B - v/2.0f);
                m_sizeMod = Max(m_sizeMod, 1.0f + d / r);
            }
        }

        void Update(const GameTime &gameTime) override
        {
            const float dt = gameTime.GetDeltaSeconds();

            if (m_target)
            {
                vec2f a = 0.4f * (m_target->GetPosition() - m_position).SafeNormalized();
                vec2f v = m_velocity + a*dt;
                if (v.Length() > 1.2f && m_velocity.Length() < v.Length()) ;
                else m_velocity = v;
            }

            m_position += m_velocity * dt;
            m_velocity -= m_velocity * 0.2f * dt;

            m_radius = Sqrt(m_size * m_sizeMod);
            m_sizeMod = 1.0f;

            if (m_size < 1.0f)
                m_size += 0.1f * dt;
        }

        static void TrySplit(Bacter *bacter, BacterArray &bacterArray, Random &random)
        {
            if (bacter->m_size >= 1.0f)
                Split(bacter, bacterArray, random);
        }

        static void Split(Bacter *bacter, BacterArray &bacterArray, Random &random)
        {
            if (bacterArray.size == MAX_BACTERS) return;

            Bacter *bacter2 = bacterArray.Obtain();
            bacter2->Setup(bacter->m_target, random);
            bacter2->SetPosition(bacter->GetPosition() + random.GetDirection()*0.1f);
            bacter2->SetRadius(bacter->GetRadius());
            bacter2->m_size = (bacter->m_size /= 2.0f);
        }

        void Render(Renderer *renderer, const BacteroidsUniforms &uniforms)
        {
            renderer->GetGraphics()->SetUniform(uniforms.m_anim, m_anim);
            renderer->SetColor(Color(1.0f, 1.2f, 0.6f));
//            renderer->DrawFilledCirlce(m_position.x, m_position.y, m_radius);
            renderer->DrawFilledCirlce(m_position.x, m_position.y, m_radius, Color(0.0f, 0.5f, 0.5f, 1.0f));
        }

    private:
        float m_anim;
        float m_size, m_sizeMod;
        const GameObject *m_target;
        int m_health;
    };

} // bact

#endif // H_BACT_BACTER_H
