
#ifndef H_BACT_BACTER_H
#define H_BACT_BACTER_H

#include "../application/GameTime.h"

#include "../renderer/Renderer.h"
#include "../graphics/Graphics.h"

#include "../math/Math.h"
#include "../math/Random.h"

namespace bact
{

    using namespace rob;

    vec4f Normalize(const vec4f &v)
    {
        float len = v.Length();
        return (len > 0.1f) ? v/len : v;
    }

    float Distance(const vec4f &a, const vec4f &b)
    { return (b - a).Length(); }

    class Target
    {
    public:
        void SetPosition(float x, float y)
        { m_position = vec4f(x, y, 0.0f, 1.0f); }

        vec4f GetPosition() const
        { return m_position; }

    private:
        vec4f m_position;
    };

    class Bacter
    {
    public:
        Bacter()
            : m_position(0.0f, 0.0f, 0.0f, 1.0f)
            , m_velocity(0.0f, 0.0f, 0.0f, 0.0f)
            , m_radius(0.0f)
            , m_anim(0.0f)
            , m_clone(0)
            , m_r0(0.5f), m_r1(0.0f)
            , m_target(0)
        { }

        void SetPosition(float x, float y)
        { m_position = vec4f(x, y, 0.0f, 1.0f); }
        void SetRadius(float r)
        { m_radius = r; }
        void SetVelocity(float x, float y)
        { m_velocity = vec4f(x, y, 0.0f, 0.0f); }
        void SetAnim(float anim)
        { m_anim = anim; }
        void SetTarget(Target *target)
        { m_target = target; }

        vec4f GetPosition() const
        { return m_position; }
        float GetRadius() const
        { return m_radius; }

        void AddVelocity(const vec4f &v)
        { m_velocity += v; }

        void DoCollision(Bacter *b)
        {
            vec4f A  = GetPosition();
            float Ar = GetRadius();
            vec4f B  = b->GetPosition();
            float Br = b->GetRadius();

            vec4f BA = A - B;
            float r = Ar + Br;
            float d = r - BA.Length();
//            if (d > r * 0.4f)
//            {
//                vec4f v = Normalize(BA) * d/16.0f;
//                AddVelocity(v);
//                b->AddVelocity(-v);
//            }
//            else
                if (d > 0.0f)
            {
                vec4f v = Normalize(BA) * d/16.0f;
                AddVelocity(v);
                b->AddVelocity(-v);
//                vec4f v = Normalize(BA) * d/4.0f;
                m_position = A + v;
                b->m_position = B - v;
            }
        }

        void Update(const GameTime &gameTime)
        {
            const float ACCEL = 0.5f;
            const float FRICT = 0.5f;
            const float GROWTH = 0.1f;

            const float dt = gameTime.GetDeltaSeconds();

            if (m_target)
            {
                vec4f d = Normalize(m_target->GetPosition() - m_position);
                m_velocity = m_velocity + d * ACCEL * dt;
            }

            m_position += m_velocity * dt;
            m_velocity -= m_velocity * FRICT * dt;

            if (m_clone)
            {
//                m_r0 -= LOSE * dt;
//                if (m_r0 < m_r1)
//                {
//                    m_clone = 0;
//                    m_r0 = m_r1;
//                }
                float d = Distance(m_position, m_clone->m_position);
                float r = m_radius + m_clone->m_radius;
                float t = d / r;
                if (t > 1.0f)
                {
                    m_clone = 0;
                    t = 1.0f;
                }
                m_r0 = m_rt * (1.0f-t) + m_r1 * t;
            }
            else if (!ShouldClone())
                m_r0 += GROWTH * dt;

            m_radius = Sqrt(m_r0);
        }

        bool ShouldClone()
        {
            const float CLONE_R = 1.0f;
            return (m_r0 > CLONE_R) && (m_clone == 0);
        }

        void Clone(Bacter *b, Random &random)
        {
            float r = random.GetReal(0.0f, 2.0f*PI_f);
            vec4f v(Cos(r), Sin(r), 0.0f, 0.0f);
            b->m_position = m_position + v * random.GetReal(0.1f, 0.5f);
            b->m_velocity = m_velocity;
            b->m_radius = m_radius;
            b->m_anim = m_anim;
            b->m_clone = this;
            b->m_r0 = m_r0;
            b->m_r1 = m_r0/2.0f;
            b->m_rt = m_r0;
            b->m_target = m_target;

            r = random.GetReal(0.0f, 2.0f*PI_f);
            vec4f u(Cos(r), Sin(r), 0.0f, 0.0f);

            m_clone = b;
            m_rt = m_r0;
            m_r1 = m_r0/2.0f;
            m_position += u * random.GetReal(0.1f, 0.5f);
        }

        void Render(Renderer *renderer, UniformHandle anim)
        {
            renderer->GetGraphics()->SetUniform(anim, m_anim);
            renderer->SetColor(Color(1.0f, 1.2f, 0.6f));
//            renderer->DrawFilledCirlce(m_position.x, m_position.y, m_radius);
            renderer->DrawFilledCirlce(m_position.x, m_position.y, m_radius, Color(0.0f, 0.5f, 0.5f, 1.0f));
        }

    private:
        vec4f m_position;
        vec4f m_velocity;
        float m_radius;
        float m_anim;
        Bacter *m_clone;
        float m_r0, m_r1, m_rt;
        Target *m_target;
    };

} // bact

#endif // H_BACT_BACTER_H
