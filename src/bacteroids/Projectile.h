
#ifndef H_BACT_PROJECTILE_H
#define H_BACT_PROJECTILE_H

#include "GameObject.h"

namespace bact
{

    class BACT_GAME_OBJECT(Projectile, 3)
    {
    public:
        Projectile();

        void SetAlive(bool alive)
        { m_alive = alive; }

        void Update(const GameTime &gameTime, const Rect &playArea) override;

        void Render(Renderer *renderer, const BacteroidsUniforms &uniforms) override;
    };

} // bact

#endif // H_BACT_PROJECTILE_H

