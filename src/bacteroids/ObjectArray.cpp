
#include "ObjectArray.h"

namespace bact
{

    ObjectArray::~ObjectArray()
    {
        RemoveAll();
    }

    void ObjectArray::Init(LinearAllocator &alloc)
    {
        m_size = 0;
        m_bacterCount = 0;
        m_projectileCount = 0;

        m_objects = alloc.AllocateArray<GameObject*>(MAX_OBJECTS);
        const size_t bacterPoolSize = GetArraySize<Bacter>(MAX_BACTERS);
        m_bacterPool.SetMemory(alloc.AllocateArray<Bacter>(MAX_BACTERS), bacterPoolSize);
        const size_t projectilePoolSize = GetArraySize<Projectile>(MAX_PROJECTILES);
        m_projectilePool.SetMemory(alloc.AllocateArray<Projectile>(MAX_PROJECTILES), projectilePoolSize);
    }

    GameObject *ObjectArray::operator[] (size_t i)
    {
        ROB_ASSERT(i < m_size);
        return m_objects[i];
    }

    Bacter *ObjectArray::ObtainBacter()
    {
        ROB_ASSERT(m_bacterCount < MAX_BACTERS);
        Bacter *bacter = m_bacterPool.Obtain();
        ROB_ASSERT(bacter != nullptr);
        m_objects[m_size++] = bacter;
        m_bacterCount++;
        return bacter;
    }

    Projectile *ObjectArray::ObtainProjectile()
    {
        ROB_ASSERT(m_projectileCount < MAX_PROJECTILES);
        Projectile *proj = m_projectilePool.Obtain();
        ROB_ASSERT(proj != nullptr);
        m_objects[m_size++] = proj;
        m_projectileCount++;
        return proj;
    }

    void ObjectArray::Remove(size_t i)
    {
        ROB_ASSERT(i < m_size); // means also "m_size > 0"

        GameObject *obj = m_objects[i];

        switch (obj->GetType())
        {
        case Bacter::TYPE:
            m_bacterPool.Return((Bacter*)obj);
            m_bacterCount--;
            break;

        case Projectile::TYPE:
            m_projectilePool.Return((Projectile*)obj);
            m_projectileCount--;
            break;

        default:
            break;
        }

        m_size--;
        m_objects[i] = m_objects[m_size];
    }

    void ObjectArray::RemoveAll()
    {
        while (m_size > 0)
            Remove(0);
    }

} // bact
