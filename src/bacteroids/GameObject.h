
#ifndef H_BACT_GAME_OBJECT_H
#define H_BACT_GAME_OBJECT_H

#include "../application/GameTime.h"
#include "../math/Math.h"

namespace bact
{

    using rob::vec4f;
    using rob::GameTime;
    using rob::Renderer;

    class GameObject
    {
    public:
        GameObject(int type)
            : m_type(type)
            , m_position(0.0f)
            , m_velocity(vec4f::Zero)
            , m_radius(1.0f)
            , m_alive(true)
        { }

        virtual ~GameObject() { }

        int GetType() const
        { return m_type; }

        void SetPosition(float x, float y)
        { m_position = vec4f(x, y, 0.0f, 1.0f); }
        void SetPosition(const vec4f &p)
        { m_position = vec4f(p.x, p.y, 0.0f, 1.0f); }
        vec4f GetPosition() const
        { return m_position; }

        void SetRadius(float r)
        { m_radius = r; }
        float GetRadius() const
        { return m_radius; }

        void SetVelocity(float x, float y)
        { m_velocity = vec4f(x, y, 0.0f, 0.0f); }
        vec4f GetVelocity() const
        { return m_velocity; }

        void AddVelocity(const vec4f &v)
        { m_velocity += v; }

        bool IsAlive() const
        { return m_alive; }

        virtual void Update(const GameTime &gameTime) { }
        virtual void Render(Renderer *renderer) { }

    protected:
        int m_type;
        vec4f m_position;
        vec4f m_velocity;
        float m_radius;
        bool m_alive;

    };

} // bact

#endif // H_BACT_GAME_OBJECT_H

