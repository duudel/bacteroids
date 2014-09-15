
#include "BacteroidsGame.h"
#include "../application/GameState.h"
#include "../application/MicroTicker.h"
#include "../application/VirtualTime.h"

#include "../renderer/Renderer.h"

#include "../math/Math.h"
#include "../math/Projection.h"
#include "../math/Random.h"

#include "../Log.h"

namespace bact
{

    using namespace rob;

    class Bacter
    {
    public:
        Bacter()
            : m_position(0.0f, 0.0f, 0.0f, 1.0f)
            , m_radius(100.0f)
        { }

        void SetPosition(float x, float y)
        { m_position = vec4f(x, y, 0.0f, 1.0f); }
        void SetRadius(float r)
        { m_radius = r; }

        void Update(const GameTime &gameTime)
        {

        }

        void Render(Renderer *renderer)
        {
            renderer->SetColor(Color(0.8f, 1.0f, 0.5f));
            renderer->DrawFilledCirlce(m_position.x, m_position.y, m_radius, Color(0.6f, 0.6f, 0.2f, 0.2f));
        }
    private:
        vec4f m_position;
        vec4f m_velocity;
        float m_radius;
    };

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
            m_bactrer.Render(m_renderer);

//            m_renderer->SetColor(Color(0.85f, 0.4f, 0.0f));
//            m_renderer->DrawFilledCirlce(0.0f, 0.0f, 1.01f * radius);
//            m_renderer->SetColor(Color(0.8f, 0.0f, 0.0f));
//            m_renderer->DrawFilledCirlce(0.0f, 0.0f, 1.0f * radius, Color(1.0f, 0.5f, 0.0f, 1.0f));
//            m_renderer->SetColor(Color(1.0f, 0.5f, 0.0f));
//            m_renderer->DrawFilledCirlce(0.0f, 0.0f, 0.9f * radius, Color(0.5f, 0.05f, 0.05f, 0.5f));
        }
    private:
        MasterCache *m_cache;
        Renderer *m_renderer;

        MicroTicker m_ticker;
        VirtualTime m_time;
        Random m_random;

        float radius;

        Bacter m_bactrer;
    };

    bool Bacteroids::Initialize()
    {
        ChangeState<BacteroidsState>(m_cache, m_renderer);
        return true;
    }

} // bact
