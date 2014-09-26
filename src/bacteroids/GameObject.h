
#ifndef H_BACT_GAME_OBJECT_H
#define H_BACT_GAME_OBJECT_H

#include "Uniforms.h"

#include "../math/Math.h"

namespace rob
{
    class GameTime;
    class Renderer;
} // rob

namespace bact
{

    using namespace rob;

    class GameObject
    {
    public:
        GameObject(int type)
            : m_type(type)
            , m_position(0.0f)
            , m_velocity(vec2f::Zero)
            , m_radius(1.0f)
            , m_alive(true)
        { }

        virtual ~GameObject() { }

        int GetType() const
        { return m_type; }

        void SetPosition(float x, float y)
        { m_position = vec2f(x, y); }
        void SetPosition(const vec2f &p)
        { m_position = p; }
        vec2f GetPosition() const
        { return m_position; }

        void SetRadius(float r)
        { m_radius = r; }
        float GetRadius() const
        { return m_radius; }

        void SetVelocity(float x, float y)
        { m_velocity = vec2f(x, y); }
        vec2f GetVelocity() const
        { return m_velocity; }

        void AddVelocity(const vec2f &v)
        { m_velocity += v; }

        bool IsAlive() const
        { return m_alive; }

        bool IsDead() const
        { return !m_alive; }

        virtual void Update(const GameTime &gameTime) { }
        virtual void Render(Renderer *renderer, const BacteroidsUniforms &uniforms) { }

    protected:
        int m_type;
        vec2f m_position;
        vec2f m_velocity;
        float m_radius;
        bool m_alive;
    };

} // bact

#endif // H_BACT_GAME_OBJECT_H

