
#include "BacteroidsGame.h"
#include "../application/GameState.h"

#include "../renderer/Renderer.h"

#include "../math/Math.h"

namespace bact
{

    using namespace rob;

    class BacteroidsState : public GameState
    {
    public:
        BacteroidsState(MasterCache *cache, Renderer *renderer)
            : m_cache(cache)
            , m_renderer(renderer)
        { }

        void Update(const GameTime &gameTime) override
        {

        }

        void Render(const GameTime &gameTime) override
        {
            m_renderer->SetColor(Color(1.0f, 0.0f, 0.0f));
            m_renderer->DrawFilledCirlce(0.0f, 0.0f, 1.0f);
            m_renderer->SetColor(Color(1.0f, 0.5f, 0.0f));
            m_renderer->DrawFilledCirlce(0.0f, 0.0f, 0.9f, Color(0.5f, 0.05f, 0.05f, 0.5f));
            m_renderer->SetColor(Color(1.0f, 1.0f, 0.0f));
            m_renderer->DrawCirlce(0.0f, 0.0f, 1.0f);
        }
    private:
        MasterCache *m_cache;
        Renderer *m_renderer;
    };

    bool Bacteroids::Initialize()
    {
        ChangeState<BacteroidsState>(m_cache, m_renderer);
        return true;
    }

} // bact
