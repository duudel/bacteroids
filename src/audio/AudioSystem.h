
#ifndef H_ROB_AUDIO_SYSTEM_H
#define H_ROB_AUDIO_SYSTEM_H

#include "../memory/Pool.h"

namespace rob
{

    class LinearAllocator;

    struct Sound;

    typedef unsigned int SoundHandle;
    static const SoundHandle InvalidSound = ~0;

    class AudioSystem
    {
    public:
        AudioSystem(LinearAllocator &alloc);
        ~AudioSystem();

        SoundHandle LoadSound(const char * const filename);
        void UnloadSound(SoundHandle sound);

        void SetMute(bool mute);
        void ToggleMute();
        bool IsMuted() const;

        void PlaySound(SoundHandle sound, float volume = 1.0f);


    private:
        struct Buffer
        {
            unsigned int buffer;
        };

        struct Channel
        {
            unsigned int source;
        };

        static const size_t MAX_CHANNELS = 16;

    private:
        Pool<Sound> m_sounds;
        Pool<Buffer> m_buffers;
        Channel m_channels[MAX_CHANNELS];
        bool m_muted;
    };

} // rob

#endif // H_ROB_AUDIO_SYSTEM_H
