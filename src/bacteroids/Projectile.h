
#ifndef H_BACT_PROJECTILE_H
#define H_BACT_PROJECTILE_H

#include "../application/GameTime.h"
#include "../math/Math.h"

namespace bact
{

    class Projectile
    {
    public:
        Projectile()
            : m_position(0.0f)
            , m_velocity(vec4f::Zero)
            , m_radius(0.2f)
            , m_alive(true)
        { }

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

        void Update(const GameTime &gameTime)
        {
            const float dt = gameTime.GetDeltaSeconds();
            m_position += m_velocity * dt;

             // This kills projectiles that are too far from the origin
            if (m_position.Length() > 100.0f)
                m_alive = false;
        }

        bool IsAlive() const
        { return m_alive; }

        void Render(Renderer *renderer)
        {
            renderer->SetColor(Color(1.0f, 1.0f, 1.6f));
            renderer->DrawFilledCirlce(m_position.x, m_position.y, m_radius, Color(0.2f, 0.5f, 0.5f, 0.5f));
        }

    private:
        vec4f m_position;
        vec4f m_velocity;
        float m_radius;
        bool m_alive;
    };


    static const size_t MAX_PROJECTILES = 200;

    struct ProjectileArray
    {
        size_t size;

        ~ProjectileArray()
        {
            RemoveAll();
        }

        void Init(LinearAllocator &alloc)
        {
            size = 0;

            m_projectiles = alloc.AllocateArray<Projectile*>(MAX_PROJECTILES);
            const size_t poolSize = GetArraySize<Projectile>(MAX_PROJECTILES);
            m_projectilePool.SetMemory(alloc.AllocateArray<Projectile>(MAX_PROJECTILES), poolSize);
        }

        Projectile *Obtain()
        {
            ROB_ASSERT(size < MAX_PROJECTILES);
            Projectile *proj = m_projectilePool.Obtain();
            ROB_ASSERT(proj != nullptr);
            m_projectiles[size++] = proj;
            return proj;
        }

        void Remove(size_t i)
        {
            ROB_ASSERT(i < size); // means also "size > 0"
            m_projectilePool.Return(m_projectiles[i]);
            if (--size > 0)
                m_projectiles[i] = m_projectiles[size];
        }

        void RemoveAll()
        {
            for (size_t i = 0; i < size; i++)
                m_projectilePool.Return(m_projectiles[i]);

            size = 0;
        }

        Projectile *operator[] (size_t i)
        {
            ROB_ASSERT(i < size);
            return m_projectiles[i];
        }

    private:
        Projectile **m_projectiles;
        Pool<Projectile> m_projectilePool;
    };

} // bact

#endif // H_BACT_PROJECTILE_H

