
#ifndef H_BACT_GAME_OBJECT_H
#define H_BACT_GAME_OBJECT_H

#include "Uniforms.h"

#include "../math/Math.h"

#define BACT_GAME_OBJECT(ClassName, TypeID) \
    ClassName : public BacteroidsGameObject<TypeID>

#define BACT_GAME_OBJECT_SHADER(ClassName) \
    template <> ShaderProgramHandle BacteroidsGameObject<ClassName::TYPE>::shader

namespace rob
{
    class GameTime;
    class Renderer;
} // rob

namespace bact
{

    using namespace rob;

    struct Rect
    {
        vec2f p0, p1;

        Rect() : p0(), p1() { }
        Rect(float x0, float y0, float x1, float y1)
            : p0(x0, y0), p1(x1, y1) { }

        bool HasPoint(const vec2f &p) const
        {
            return (p.x >= p0.x && p.x <= p1.x) &&
                (p.y >= p0.y && p.y <= p1.y);
        }
        bool HasCircle(const vec2f &p, const float radius) const
        {
            return (p.x >= p0.x + radius && p.x <= p1.x - radius) &&
                (p.y >= p0.y + radius && p.y <= p1.y - radius);
        }
    };

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

        virtual void Update(const GameTime &gameTime, const Rect &playArea) { }
        virtual void Render(Renderer *renderer, const BacteroidsUniforms &uniforms) { }

        virtual ShaderProgramHandle GetShader() = 0;

    protected:
        int m_type;
        vec2f m_position;
        vec2f m_velocity;
        float m_radius;
        bool m_alive;
    };

    template <int TypeID>
    class BacteroidsGameObject : public GameObject
    {
    public:
        static const int TYPE = TypeID;
        static ShaderProgramHandle shader;
    public:
        BacteroidsGameObject()
            : GameObject(TYPE)
        { }

        ShaderProgramHandle GetShader() override
        { return shader; }
    };

} // bact

#endif // H_BACT_GAME_OBJECT_H

