
#include "SoundCache.h"

#include "../Log.h"

namespace rob
{

    SoundCache::SoundCache(AudioSystem *audio)
        : m_audio(audio)
    { }

    ROB_DEFINE_RESOURCE_CACHE_DTOR(SoundCache)

    SoundHandle SoundCache::Load(const char * const filename)
    {
        return m_audio->LoadSound(filename);
    }

    void SoundCache::Unload(SoundHandle sound)
    {
        m_audio->UnloadSound(sound);
    }

} // rob
