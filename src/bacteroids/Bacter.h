
#ifndef H_BACT_BACTER_H
#define H_BACT_BACTER_H

#include "GameObject.h"

#include "../renderer/Renderer.h"
#include "../graphics/Graphics.h"

#include "../math/Random.h"

namespace bact
{

    using namespace rob;

    vec4f Normalize(const vec4f &v)
    {
        float len = v.Length();
        return (len > 0.1f) ? v/len : v;
    }

    class Bacter : public GameObject
    {
    public:
        static const int TYPE = 2;
    public:
        Bacter()
            : GameObject(TYPE)
            , m_anim(0.0f)
            , m_r0(0.5f), m_r1(0.0f)
            , m_target(nullptr)
        {
            SetRadius(1.0f);
        }

        void SetAnim(float anim)
        { m_anim = anim; }

        void SetTarget(const GameObject *target)
        { m_target = target; }

        void DoCollision(Bacter *b)
        {
            vec4f A  = GetPosition();
            float Ar = GetRadius();
            vec4f B  = b->GetPosition();
            float Br = b->GetRadius();

            vec4f BA = A - B;
            float r = Ar + Br;
            float d = r - BA.Length();
            if (d > 0.0f)
            {
//                if (d > 0.3f)
                {
                    vec4f v = Normalize(BA) * d/8.0f;
                    AddVelocity(v);
                    b->AddVelocity(-v);
                    SetPosition(A + v/2.0f);
                    b->SetPosition(B - v/2.0f);
                }
//                else
//                {
//                    vec4f v = Normalize(BA) * d/8.0f;
//                    SetPosition(A + v);
//                    b->SetPosition(B - v);
//                }
                m_r1 = Max(m_r1, d / r);
                b->m_r1 = Max(b->m_r1, m_r1);
            }
        }

        void Update(const GameTime &gameTime) override
        {
            const float dt = gameTime.GetDeltaSeconds();

            if (m_target)
            {
                vec4f a = 0.4f * Normalize(m_target->GetPosition() - m_position);
                vec4f v = m_velocity + a*dt;
                if (v.Length() > 1.2f && m_velocity.Length() < v.Length()) ;
                else m_velocity = v;
            }

            m_position += m_velocity * dt;
            m_velocity -= m_velocity * 0.2f * dt;

            m_radius = Sqrt(m_r0+m_r0*m_r1);
            m_r1 = 0.0f;

            if (!ShouldClone())
                m_r0 += 0.1f * dt;
        }

        bool ShouldClone() const
        {
            return (m_r0 > 1.0f);
        }

        void Clone(Bacter *b, Random &random)
        {
            b->m_position = m_position + random.GetDirection()*0.1f;
//            b->m_velocity = m_velocity;
            b->m_radius = m_radius;
            b->m_anim = random.GetReal(0.0, 2.0 * PI_d); //m_anim + 0.637f;
            b->m_r0 = m_r0/2.0f;
            b->m_target = m_target;

            m_r0 = m_r0/2.0f;
        }

        void Render(Renderer *renderer, UniformHandle anim)
        {
            renderer->GetGraphics()->SetUniform(anim, m_anim);
            renderer->SetColor(Color(1.0f, 1.2f, 0.6f));
//            renderer->DrawFilledCirlce(m_position.x, m_position.y, m_radius);
            renderer->DrawFilledCirlce(m_position.x, m_position.y, m_radius, Color(0.0f, 0.5f, 0.5f, 1.0f));
        }

    private:
        float m_anim;
        float m_r0, m_r1;
        const GameObject *m_target;
    };

} // bact

#endif // H_BACT_BACTER_H
