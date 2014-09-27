
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
            , m_plDeathSound(InvalidSound)
            , m_bactDeathSound(InvalidSound)
        { }

        void Init(AudioSystem &audio, MasterCache &cache)
        {
            m_audio = &audio;
            m_shootSound = cache.GetSound("Blip_Select7.wav");
            m_plDeathSound = cache.GetSound("Explosion5.wav");
            m_bactDeathSound = cache.GetSound("Randomize6.wav");
//            m_bactDeathSound = cache.GetSound("Hit_Hurt.wav");
        }

        void PlayShootSound()
        {
            m_audio->PlaySound(m_shootSound, 1.0f);
        }

        void PlayPlayerDeathSound()
        {
            m_audio->PlaySound(m_plDeathSound, 0.5f);
        }

        void PlayBacterDeathSound()
        {
            m_audio->PlaySound(m_bactDeathSound, 0.5f);
        }

    private:
        AudioSystem *m_audio;
//        MasterCache *m_cache;

        SoundHandle m_shootSound;
        SoundHandle m_plDeathSound;
        SoundHandle m_bactDeathSound;
    };

} // bact

#endif // H_BACT_SOUND_PLAYER_H
