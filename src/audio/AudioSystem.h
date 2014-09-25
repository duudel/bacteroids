
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
        Pool<Sound> m_sounds;
        bool m_muted;
    };

} // rob

#endif // H_ROB_AUDIO_SYSTEM_H
