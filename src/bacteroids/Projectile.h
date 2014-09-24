
#ifndef H_BACT_PROJECTILE_H
#define H_BACT_PROJECTILE_H

#include "GameObject.h"

namespace bact
{

    class Projectile : public GameObject
    {
    public:
        static const int TYPE = 3;
    public:
        Projectile();

        void SetAlive(bool alive)
        { m_alive = alive; }

        void Update(const GameTime &gameTime) override;

        void Render(Renderer *renderer, const BacteroidsUniforms &uniforms) override;
    };

} // bact

#endif // H_BACT_PROJECTILE_H

