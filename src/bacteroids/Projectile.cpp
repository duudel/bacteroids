
#include "Projectile.h"

#include "../renderer/Renderer.h"

#include "../application/GameTime.h"

namespace bact
{

    using namespace rob;

    Projectile::Projectile()
    {
        SetRadius(0.2f);
    }

    void Projectile::Update(const GameTime &gameTime, const Rect &playArea)
    {
        const float dt = gameTime.GetDeltaSeconds();
        m_position += m_velocity * dt;

        // NOTE: Negative radius means the projectile can be
        // outside of the play area by length of 2*radius.
        if (!playArea.HasCircle(m_position, -m_radius * 2))
            m_alive = false;
    }

    void Projectile::Render(Renderer *renderer, const BacteroidsUniforms &uniforms)
    {
        renderer->SetColor(Color(1.0f, 1.0f, 1.6f));
        renderer->DrawFilledCirlce(m_position.x, m_position.y, m_radius, Color(0.2f, 0.5f, 0.5f, 0.5f));
    }

} // bact
