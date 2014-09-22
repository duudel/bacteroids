
#ifndef H_BACT_PROJECTILE_H
#define H_BACT_PROJECTILE_H

#include "GameObject.h"
#include "Bacter.h"
#include "../memory/Pool.h"

namespace bact
{

    class Projectile : public GameObject
    {
    public:
        static const int TYPE = 3;
    public:
        Projectile()
            : GameObject(TYPE)
        {
            SetRadius(0.2f);
        }

        void Update(const GameTime &gameTime) override
        {
            const float dt = gameTime.GetDeltaSeconds();
            m_position += m_velocity * dt;

             // This kills projectiles that are too far from the origin
            if (m_position.Length() > 100.0f)
                m_alive = false;
        }

        void Render(Renderer *renderer, const BacteroidsUniforms &uniforms) override
        {
            renderer->SetColor(Color(1.0f, 1.0f, 1.6f));
            renderer->DrawFilledCirlce(m_position.x, m_position.y, m_radius, Color(0.2f, 0.5f, 0.5f, 0.5f));
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
                m_alive = false;
                b->Hit();
            }
        }
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

