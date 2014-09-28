
#ifndef H_BACT_PLAYER_H
#define H_BACT_PLAYER_H

#include "GameObject.h"

namespace bact
{

    using namespace rob;

    class ObjectArray;
    class SoundPlayer;
    class Input;

    class BACT_GAME_OBJECT(Player, 1)
    {
    public:
        Player();

        void TakeHit(SoundPlayer &sounds);
        float GetHealth() const;

        void Update(const GameTime &gameTime, const Input &input, ObjectArray &projectiles, SoundPlayer &sounds);

        void Cooldown(const GameTime &gameTime);
        void Shoot(const GameTime &gameTime, ObjectArray &projectiles, SoundPlayer &sounds);

        void Render(Renderer *renderer, const BacteroidsUniforms &uniforms) override;

    private:
        vec2f m_direction;
        float m_health;
        float m_cooldown;
        float m_dmgSoundTimer;
    };

} // bact

#endif // H_BACT_PLAYER_H

