
#ifndef H_BACT_PROJECTILE_H
#define H_BACT_PROJECTILE_H

#include "GameObject.h"
//#include "Bacter.h"
//#include "../memory/Pool.h"

namespace bact
{

    class Projectile : public GameObject
    {
    public:
        static const int TYPE = 3;
    public:
        Projectile();

        void SetAlive(bool alive)
        { m_alive = alive; }

        void Update(const GameTime &gameTime) override;

        void Render(Renderer *renderer, const BacteroidsUniforms &uniforms) override;
    };


//    static const size_t MAX_PROJECTILES = 200;

//    struct ProjectileArray
//    {
//        size_t size;
//
//        ~ProjectileArray()
//        {
//            RemoveAll();
//        }
//
//        void Init(LinearAllocator &alloc)
//        {
//            size = 0;
//
//            m_projectiles = alloc.AllocateArray<Projectile*>(MAX_PROJECTILES);
//            const size_t poolSize = GetArraySize<Projectile>(MAX_PROJECTILES);
//            m_projectilePool.SetMemory(alloc.AllocateArray<Projectile>(MAX_PROJECTILES), poolSize);
//        }
//
//        Projectile *Obtain()
//        {
//            ROB_ASSERT(size < MAX_PROJECTILES);
//            Projectile *proj = m_projectilePool.Obtain();
//            ROB_ASSERT(proj != nullptr);
//            m_projectiles[size++] = proj;
//            return proj;
//        }
//
//        void Remove(size_t i)
//        {
//            ROB_ASSERT(i < size); // means also "size > 0"
//            m_projectilePool.Return(m_projectiles[i]);
//            if (--size > 0)
//                m_projectiles[i] = m_projectiles[size];
//        }
//
//        void RemoveAll()
//        {
//            for (size_t i = 0; i < size; i++)
//                m_projectilePool.Return(m_projectiles[i]);
//
//            size = 0;
//        }
//
//        Projectile *operator[] (size_t i)
//        {
//            ROB_ASSERT(i < size);
//            return m_projectiles[i];
//        }
//
//    private:
//        Projectile **m_projectiles;
//        Pool<Projectile> m_projectilePool;
//    };

} // bact

#endif // H_BACT_PROJECTILE_H

