
#include "BacteroidsGame.h"
#include "../application/GameState.h"
#include "../application/MicroTicker.h"
#include "../application/VirtualTime.h"

#include "../renderer/Renderer.h"

#include "../math/Math.h"
#include "../math/Projection.h"

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
            m_time.Restart(m_ticker);
            m_random.Seed(m_ticker.GetTicks());
        }

        void OnResize(int w, int h) override
        {
            int x0 = -w / 2;
            int x1 = w / 2;
            int y0 = -h / 2;
            int y1 = h / 2;

            m_renderer->SetProjection(Projection_Orthogonal_lh(x0, x1, y0, y1, -1, 1));
        }

        void OnKeyPress(Key key, uint32_t mods) override
        {
            if (m_time.IsPaused())
                m_time.Resume(m_ticker);
            else
                m_time.Pause();
        }

        void Update(const GameTime &gameTime) override
        {
            m_time.Update(m_ticker);
            radius = 0.9f + Sin(m_time.GetTime() * 3.14) * 0.1f;
//            radius *= 100.0f;
            radius = 100.0f;
        }

        void Render(const GameTime &gameTime) override
        {
            m_renderer->SetTime(m_time.GetTime());
//            m_renderer->SetColor(Color(0.5f, 0.05f, 0.05f));
//            m_renderer->DrawFilledCirlce(0.0f, 0.0f, 1.0f * radius, Color(1.0f, 0.5f, 0.0f, 0.5f));
            m_renderer->SetColor(Color(0.85f, 0.4f, 0.0f));
            m_renderer->DrawFilledCirlce(0.0f, 0.0f, 1.01f * radius);
            m_renderer->SetColor(Color(0.8f, 0.0f, 0.0f));
            m_renderer->DrawFilledCirlce(0.0f, 0.0f, 1.0f * radius, Color(1.0f, 0.5f, 0.0f, 1.0f));
            m_renderer->SetColor(Color(1.0f, 0.5f, 0.0f));
            m_renderer->DrawFilledCirlce(0.0f, 0.0f, 0.9f * radius, Color(0.5f, 0.05f, 0.05f, 0.5f));
//            m_renderer->SetColor(Color(1.0f, 0.8f, 0.2f));
//            m_renderer->DrawCirlce(0.0f, 0.0f, 1.0f * radius);
        }
    private:
        MasterCache *m_cache;
        Renderer *m_renderer;

        MicroTicker m_ticker;
        VirtualTime m_time;
        Random m_random;



        float radius;
    };

    bool Bacteroids::Initialize()
    {
        ChangeState<BacteroidsState>(m_cache, m_renderer);
        return true;
    }

} // bact
