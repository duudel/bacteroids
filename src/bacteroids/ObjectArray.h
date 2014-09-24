
#ifndef H_BACT_OBJECT_ARRAY_H
#define H_BACT_OBJECT_ARRAY_H

#include "Bacter.h"
#include "Projectile.h"

#include "../memory/Pool.h"

#include "../Types.h"

namespace rob
{
    class LinearAllocator;
} // rob

namespace bact
{

    using rob::LinearAllocator;

    static const size_t MAX_BACTERS = 500;
    static const size_t MAX_PROJECTILES = 200;
    static const size_t MAX_OBJECTS = MAX_BACTERS + MAX_PROJECTILES + 1;

    class ObjectArray
    {
    public:
        ~ObjectArray();

        void Init(LinearAllocator &alloc);

        GameObject *operator[] (size_t i);

        size_t Size() const
        { return m_size; }
        bool CanObtainBacter() const
        { return (m_bacterCount < MAX_BACTERS); }
        bool CanObtainProjectile() const
        { return (m_projectileCount < MAX_PROJECTILES); }

        void AddObject(GameObject *obj)
        { m_objects[m_size++] = obj; }

        Bacter *ObtainBacter();
        Projectile *ObtainProjectile();

        void Remove(size_t i);
        void RemoveAll();

    private:
        size_t m_size;
        size_t m_bacterCount;
        size_t m_projectileCount;

        GameObject **m_objects;
        Pool<Bacter> m_bacterPool;
        Pool<Projectile> m_projectilePool;
    };

} // bact

#endif // H_BACT_OBJECT_ARRAY_H
