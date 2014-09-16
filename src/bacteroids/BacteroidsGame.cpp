
#include "BacteroidsGame.h"

#include "../application/GameState.h"
#include "../application/MicroTicker.h"
#include "../application/VirtualTime.h"

#include "../renderer/Renderer.h"
#include "../graphics/Graphics.h"

#include "../math/Math.h"
#include "../math/Projection.h"
#include "../math/Random.h"

#include "../Log.h"

#include "Shaders.h"

namespace bact
{

    using namespace rob;

    class Bacter
    {
    public:
        Bacter()
            : m_position(0.0f, 0.0f, 0.0f, 1.0f)
            , m_radius(55.0f)
        { }

        void SetPosition(float x, float y)
        { m_position = vec4f(x, y, 0.0f, 1.0f); }
        void SetRadius(float r)
        { m_radius = r; }
        void SetVelocity(float x, float y)
        { m_velocity = vec4f(x, y, 0.0f, 0.0f); }

        void Update(const GameTime &gameTime)
        {
            m_position += m_velocity * gameTime.GetDeltaSeconds();
        }

        void Render(Renderer *renderer)
        {
            renderer->SetColor(Color(1.0f, 1.2f, 0.6f));
//            renderer->DrawFilledCirlce(m_position.x, m_position.y, m_radius);
            renderer->DrawFilledCirlce(m_position.x, m_position.y, m_radius, Color(0.0f, 0.5f, 0.5f, 1.0f));
        }

    private:
        vec4f m_position;
        vec4f m_velocity;
        float m_radius;
    };

    static const size_t MAX_BACTERS = 1000;

    struct BacterArray
    {
        size_t size;

        ~BacterArray()
        {
            RemoveAll();
        }

        void Init(LinearAllocator &alloc)
        {
            size = 0;

            m_bacters = alloc.AllocateArray<Bacter*>(MAX_BACTERS);
            const size_t poolSize = GetArraySize<Bacter>(MAX_BACTERS);
            m_bacterPool.SetMemory(alloc.AllocateArray<Bacter>(MAX_BACTERS), poolSize);
        }

        Bacter *Obtain()
        {
            ROB_ASSERT(size < MAX_BACTERS);
            m_bacters[size] = m_bacterPool.Obtain();
            return m_bacters[size++];
        }

        void Remove(size_t i)
        {
            ROB_ASSERT(i < size); // means also "size > 0"
            m_bacterPool.Return(m_bacters[i]);
            if (--size > 0)
                m_bacters[i] = m_bacters[size];
        }

        void RemoveAll()
        {
            for (size_t i = 0; i < size; i++)
                m_bacterPool.Return(m_bacters[i]);

            size = 0;
        }

        Bacter *operator[] (size_t i)
        {
            ROB_ASSERT(i < size);
            return m_bacters[i];
        }

    private:
        Bacter **m_bacters;
        Pool<Bacter> m_bacterPool;
    };

    struct Viewport
    {
        int x, y, w, h;
    };

    const float PLAY_AREA_W         = 800.0f;
    const float PLAY_AREA_H         = 600.0f;
    const float PLAY_AREA_LEFT      = -PLAY_AREA_W / 2.0f;
    const float PLAY_AREA_RIGHT     = -PLAY_AREA_LEFT;
    const float PLAY_AREA_BOTTOM    = -PLAY_AREA_H / 2.0f;
    const float PLAY_AREA_TOP       = -PLAY_AREA_BOTTOM;

    class BacteroidsState : public GameState
    {
    public:
        BacteroidsState()
        {
            m_random.Seed(m_ticker.GetTicks());
        }

        bool Initialize() override
        {
            GetRenderer().SetProjection(Projection_Orthogonal_lh(PLAY_AREA_LEFT,
                                                                 PLAY_AREA_RIGHT,
                                                                 PLAY_AREA_BOTTOM,
                                                                 PLAY_AREA_TOP, -1, 1));

//            GetRenderer().GetGraphics()->SetClearColor(0.05f, 0.13f, 0.15f);
            m_bacterShader = GetRenderer().CompileShaderProgram(g_bacterShader.m_vertexShader,
                                                              g_bacterShader.m_fragmentShader);

            m_bacters.Init(GetAllocator());
//            m_bacters.Obtain()->SetVelocity(m_random.GetReal(-1.0, 1.0) * 20.0f, m_random.GetReal(-1.0, 1.0) * 20.0f);
            m_bacters.Obtain()->SetVelocity(20.0f, 20.0f);
            m_bacters.Obtain()->SetPosition(58.0f, 0.0f);
            return true;
        }

        ~BacteroidsState()
        {
            GetRenderer().GetGraphics()->DestroyShaderProgram(m_bacterShader);
        }

        void OnResize(int w, int h) override
        {
            float x_scl = w / PLAY_AREA_W;
            float y_scl = h / PLAY_AREA_H;
            float scale = (x_scl < y_scl) ? x_scl : y_scl;

            m_playAreaVp.w = scale * PLAY_AREA_W;
            m_playAreaVp.h = scale * PLAY_AREA_H;
            m_playAreaVp.x = (w - m_playAreaVp.w) / 2;
            m_playAreaVp.y = (h - m_playAreaVp.h) / 2;

            m_screenVp.x = 0;
            m_screenVp.y = 0;
            m_screenVp.w = w;
            m_screenVp.h = h;
        }

        void OnKeyPress(Key key, uint32_t mods) override
        {
            if (m_time.IsPaused())
                m_time.Resume();
            else
                m_time.Pause();
        }

        void Update(const GameTime &gameTime) override
        {
            m_time.Update();

            for (size_t i = 0; i < m_bacters.size; i++)
            {
                m_bacters[i]->Update(gameTime);
            }
        }

        void SetViewport(Viewport &vp)
        {
            GetRenderer().GetGraphics()->SetViewport(vp.x, vp.y, vp.w, vp.h);
        }

        void Render() override
        {
            SetViewport(m_playAreaVp);

            Renderer &renderer = GetRenderer();
            renderer.BindColorShader();
            renderer.SetColor(Color(0.05f, 0.13f, 0.15f));
            renderer.DrawFilledRectangle(PLAY_AREA_LEFT, PLAY_AREA_BOTTOM, PLAY_AREA_RIGHT, PLAY_AREA_TOP);
            renderer.SetTime(m_time.GetTime());

            for (size_t i = 0; i < m_bacters.size; i++)
            {
                renderer.BindShader(m_bacterShader);
                m_bacters[i]->Render(&renderer);
            }
        }
    private:
        Random m_random;

        ShaderProgramHandle m_bacterShader;

        BacterArray m_bacters;

        Viewport m_playAreaVp;
        Viewport m_screenVp;
    };

    bool Bacteroids::Initialize()
    {
        ChangeState<BacteroidsState>();
        return true;
    }

} // bact
