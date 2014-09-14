
#include "BacteroidsGame.h"
#include "../application/GameState.h"
#include "../application/MicroTicker.h"

#include "../renderer/Renderer.h"

#include "../Log.h"

#include <random>

namespace bact
{

    class Random
    {
    public:
        void Seed(uint32_t seed)
        {
            m_generator.seed(seed);
        }

        int GetInt()
        {
            std::uniform_int_distribution<int> dis;
            return dis(m_generator);
        }

        int GetInt(int a, int b)
        {
            std::uniform_int_distribution<int> dis(a, b);
            return dis(m_generator);
        }

        double GetReal()
        {
            std::uniform_real_distribution<double> dis;
            return dis(m_generator);
        }

        double GetReal(double a, double b)
        {
            std::uniform_real_distribution<double> dis(a, b);
            return dis(m_generator);
        }

    private:
        std::mt19937 m_generator;
    };

    using namespace rob;

    class BacteroidsState : public GameState
    {
    public:
        BacteroidsState(MasterCache *cache, Renderer *renderer)
            : m_cache(cache)
            , m_renderer(renderer)
        {
            m_ticker.Init();
            m_random.Seed(m_ticker.GetTicks());
        }

        void Update(const GameTime &gameTime) override
        { }

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

        MicroTicker m_ticker;
        Random m_random;
    };

    bool Bacteroids::Initialize()
    {
        ChangeState<BacteroidsState>(m_cache, m_renderer);
        return true;
    }

} // bact
