
#ifndef H_BACT_BACTER_ARRAY_H
#define H_BACT_BACTER_ARRAY_H

namespace bact
{

    static const size_t MAX_BACTERS = 1000;

    struct BacterArray
    {
        size_t size;

        ~BacterArray()
        {
            RemoveAll();
        }

        void Init(LinearAllocator &alloc)
        {
            size = 0;

            m_bacters = alloc.AllocateArray<Bacter*>(MAX_BACTERS);
            const size_t poolSize = GetArraySize<Bacter>(MAX_BACTERS);
            m_bacterPool.SetMemory(alloc.AllocateArray<Bacter>(MAX_BACTERS), poolSize);
        }

        Bacter *Obtain()
        {
            ROB_ASSERT(size < MAX_BACTERS);
            m_bacters[size] = m_bacterPool.Obtain();
            return m_bacters[size++];
        }

        void Remove(size_t i)
        {
            ROB_ASSERT(i < size); // means also "size > 0"
            m_bacterPool.Return(m_bacters[i]);
            if (--size > 0)
                m_bacters[i] = m_bacters[size];
        }

        void RemoveAll()
        {
            for (size_t i = 0; i < size; i++)
                m_bacterPool.Return(m_bacters[i]);

            size = 0;
        }

        Bacter *operator[] (size_t i)
        {
            ROB_ASSERT(i < size);
            return m_bacters[i];
        }

    private:
        Bacter **m_bacters;
        Pool<Bacter> m_bacterPool;
    };

} // bact

#endif // H_BACT_BACTER_ARRAY_H
