
#ifndef H_BACT_SOUND_PLAYER_H
#define H_BACT_SOUND_PLAYER_H

#include "../audio/AudioSystem.h"
#include "../resource/MasterCache.h"
#include "../application/GameTime.h"

namespace bact
{

    using namespace rob;

    class SoundPlayer
    {
    public:
        SoundPlayer()
            : m_audio(nullptr)
//            , m_cache(nullptr)
            , m_currentTime(0)
            , m_shootSound(InvalidSound)
            , m_plDeathSound(InvalidSound)
            , m_bactSplitSound(InvalidSound)
        { }

        void Init(AudioSystem &audio, MasterCache &cache)
        {
            m_audio = &audio;
            m_shootSound = cache.GetSound("Blip_Select7.wav");
            m_plDeathSound = cache.GetSound("Explosion5.wav");
            m_bactSplitSound = cache.GetSound("Randomize6.wav");
        }

        void UpdateTime(const GameTime &gameTime)
        {
            m_currentTime = gameTime.GetTotalMicroseconds();
        }

        void PlayShootSound()
        {
            m_audio->PlaySound(m_shootSound, 1.0f, m_currentTime);
        }

        void PlayPlayerDeathSound()
        {
            m_audio->PlaySound(m_plDeathSound, 0.5f, m_currentTime);
        }

        void PlayBacterSplitSound()
        {
            m_audio->PlaySound(m_bactSplitSound, 0.5f, m_currentTime);
        }

    private:
        AudioSystem *m_audio;
//        MasterCache *m_cache;
        Time_t m_currentTime;

        SoundHandle m_shootSound;
        SoundHandle m_plDeathSound;
        SoundHandle m_bactSplitSound;
    };

} // bact

#endif // H_BACT_SOUND_PLAYER_H
