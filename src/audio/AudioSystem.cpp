
#include "AudioSystem.h"

#include "../memory/LinearAllocator.h"

#include "../Log.h"

//#include <SDL2/SDL_mixer.h>

namespace rob
{

    struct Sound
    {
        int refCount;
        int bufferCount;
        Buffer *buffers[MAX_BUFFERS_PER_SOUND];
        DataSource *data;
        AudioSystem *audio;

        Sound()
        {
            refCount = 0;
            bufferCount = 0;
            for (int i = 0; i < MAX_BUFFERS_PER_SOUND; i++)
            {
                buffers[i] = 0;
            }
            data = 0;
            audio = 0;
        }

        ~Sound()
        {
            for (int i = 0; i < bufferCount; i++)
            {
                if (buffers[i])
                {
                    audio->ReturnBuffer(buffers[i]);
                }
            }
            if (data)
            {
                audio->CloseDataSource(data);
            }
        }

        void IncreaseRef()
        { refCount++; }
        void DecreaseRef()
        { refCount--; }

        void IncreaseBufferRef(int index)
        { buffers[index]->IncreaseRef(); }
        void DecreaseBufferRef(int index)
        { buffers[index]->DecreaseRef(); }

        bool IsLastBufferIndex(int index)
        { return index == (bufferCount - 1); }

        ALuint GetBuffer(int index)
        {
            if (buffers[index]) return buffers[index]->GetID();

            Buffer *buffer = audio->ObtainBuffer();
            data->FillBuffer(buffer, index);
            buffers[index] = buffer;
            return buffer->GetID();
        }
    };

    struct Channel
    {
        ALuint source;
        Sound *playingSound;
        int frontIndex;
        int backIndex;

        void PlaySound(Sound *sound)
        {
            playingSound = sound;
            frontIndex = 0;
            backIndex = -1;
            for (int i = 0; i < 3; i++)
            {
                if (!Queue()) break;
            }
            playingSound->IncreaseRef();
        }

        void Update()
        {
            if (playingSound == nullptr) return;

            int processed = 0;
            GetProcessed(source, &processed);

            while (processed > 0)
            {
                Dequeue(source, 1);
                playingSound->DecreaseBufferRef(frontIndex);
                frontIndex++;
                processed--;
                Queue();
            }

            if (frontIndex > backIndex)
            {
                playingSound->DecreaseRef();
                playingSound = nullptr;
            }
        }

        bool Queue()
        {
            if (playingSound->IsLastBufferIndex(backIndex)) return false;

            backIndex++;
            ALuint buffer = playingSound->GetBuffer(backIndex);
            Queue(source, 1, &buffer);
            playingSound->IncreaseBufferRef(backIndex);
            return true;
        }
    };

    AudioSystem::AudioSystem(LinearAllocator& alloc)
        : m_sounds()
        , m_muted(false)
    {


        const size_t allocSize = 1024;
        m_sounds.SetMemory(alloc.Allocate(allocSize, alignof(Sound)), allocSize);
    }

//    struct Sound
//    {
//        Mix_Chunk *chunk;
//    };
//
//    AudioSystem::AudioSystem(LinearAllocator &alloc)
//        : m_sounds()
//        , m_muted(false)
//    {
//        if (::Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) != 0)
//        {
//            log::Error("Could not open audio: ", ::Mix_GetError());
//            return;
//        }
//
//        ::Mix_AllocateChannels(16);
//
//        const size_t allocSize = 1024;
//        m_sounds.SetMemory(alloc.Allocate(allocSize, alignof(Sound)), allocSize);
//    }
//
//    AudioSystem::~AudioSystem()
//    {
//        ::Mix_AllocateChannels(0);
//        ::Mix_CloseAudio();
//    }
//
//    SoundHandle AudioSystem::LoadSound(const char * const filename)
//    {
//        Sound *sound = m_sounds.Obtain();
//
//        sound->chunk = ::Mix_LoadWAV(filename);
//        if (sound->chunk == nullptr)
//        {
//            log::Error("Could not load sound ", filename, ": ", ::Mix_GetError());
//            m_sounds.Return(sound);
//            return InvalidSound;
//        }
//
//        return m_sounds.IndexOf(sound);
//    }
//
//    void AudioSystem::UnloadSound(SoundHandle sound)
//    {
//        if (sound == InvalidSound) return;
//
//        Sound *theSound = m_sounds.Get(sound);
//        ::Mix_FreeChunk(theSound->chunk);
//        m_sounds.Return(theSound);
//    }
//
//    void AudioSystem::SetMute(bool mute)
//    { m_muted = mute; }
//
//    void AudioSystem::ToggleMute()
//    { SetMute(!IsMuted()); }
//
//    bool AudioSystem::IsMuted() const
//    { return m_muted; }
//
//    void AudioSystem::PlaySound(SoundHandle sound, float volume)
//    {
//        if (IsMuted()) return;
//        if (sound == InvalidSound) return;
//
//        Mix_Chunk *chunk = m_sounds.Get(sound)->chunk;
//        int channel = ::Mix_PlayChannel(-1, chunk, 0);
//        ::Mix_Volume(channel, MIX_MAX_VOLUME * volume);
//    }

} // rob
