
#include "AudioSystem.h"

#include "../memory/LinearAllocator.h"

#include "../Log.h"

#include <SDL2/SDL_mixer.h>

namespace rob
{

    struct Sound
    {
        Mix_Chunk *chunk;
    };

    AudioSystem::AudioSystem(LinearAllocator &alloc)
        : m_sounds()
    {
        if (::Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) != 0)
        {
            log::Error("Could not open audio: ", ::Mix_GetError());
            return;
        }

        ::Mix_AllocateChannels(16);

        const size_t allocSize = 1024;
        m_sounds.SetMemory(alloc.Allocate(allocSize, alignof(Sound)), allocSize);
    }

    AudioSystem::~AudioSystem()
    {
        ::Mix_AllocateChannels(0);
        ::Mix_CloseAudio();
    }

    SoundHandle AudioSystem::LoadSound(const char * const filename)
    {
        Sound *sound = m_sounds.Obtain();

        sound->chunk = ::Mix_LoadWAV(filename);
        if (sound->chunk == nullptr)
        {
            log::Error("Could not load sound ", filename, ": ", ::Mix_GetError());
            m_sounds.Return(sound);
            return InvalidSound;
        }

        return m_sounds.IndexOf(sound);
    }

    void AudioSystem::UnloadSound(SoundHandle sound)
    {
        if (sound == InvalidSound) return;

        Sound *theSound = m_sounds.Get(sound);
        ::Mix_FreeChunk(theSound->chunk);
        m_sounds.Return(theSound);
    }

    void AudioSystem::PlaySound(SoundHandle sound, float volume)
    {
        if (sound == InvalidSound) return;

        Mix_Chunk *chunk = m_sounds.Get(sound)->chunk;
        int channel = ::Mix_PlayChannel(-1, chunk, 0);
        ::Mix_Volume(channel, MIX_MAX_VOLUME * volume);
    }

} // rob
