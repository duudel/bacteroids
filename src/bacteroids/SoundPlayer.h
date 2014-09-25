
#ifndef H_BACT_SOUND_PLAYER_H
#define H_BACT_SOUND_PLAYER_H

#include "../audio/AudioSystem.h"
#include "../resource/MasterCache.h"

namespace bact
{

    using namespace rob;

    class SoundPlayer
    {
    public:
        SoundPlayer()
            : m_audio(nullptr)
//            , m_cache(nullptr)
            , m_shootSound(InvalidSound)
        { }

        void Init(AudioSystem &audio, MasterCache &cache)
        {
            m_audio = &audio;
            m_shootSound = cache.GetSound("Laser_Shoot6.wav");
        }

        void PlayShootSound()
        {
            m_audio->PlaySound(m_shootSound);
        }

    private:
        AudioSystem *m_audio;
//        MasterCache *m_cache;

        SoundHandle m_shootSound;
    };

} // bact

#endif // H_BACT_SOUND_PLAYER_H
