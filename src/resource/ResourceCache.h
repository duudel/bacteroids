
#ifndef H_ROB_RESOURCE_CACHE_H
#define H_ROB_RESOURCE_CACHE_H

#include "ResourceID.h"
#include <unordered_map>
#include <vector>

namespace rob
{

    template <class T>
    class ResourceCache
    {
    public:
        T Get(ResourceID id, const char * const filename)
        {
            auto it = m_resources.find(id);
            if (it != m_resources.end())
                return it->second;

            T resource = Load(filename);
            m_resources[id] = resource;
            return resource;
        }

        void UnloadAll()
        {
            for (auto it : m_resources)
                Unload(it.second);
            m_resources.clear();
        }

    protected:
        virtual T Load(const char * const filename) = 0;
        virtual void Unload(T resource) = 0;

    protected:
        std::unordered_map<uint32_t, T> m_resources;
    };

    #define ROB_DEFINE_RESOURCE_CACHE_DTOR(Class) \
        Class::~Class() \
        { \
            if (!m_resources.empty()) \
            { \
                log::Info(#Class ": Resources unloaded by destructor"); \
                UnloadAll(); \
            } \
        }

} // rob

#endif // H_ROB_RESOURCE_CACHE_H
