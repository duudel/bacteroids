
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
            m_position += m_velocity * 0.01f;
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

    class BacteroidsState : public GameState
    {
    public:
        BacteroidsState()
        {
            m_ticker.Init();
            m_time.Restart(m_ticker);
            m_random.Seed(m_ticker.GetTicks());
        }

        bool Initialize() override
        {
            GetRenderer().GetGraphics()->SetClearColor(0.05f, 0.13f, 0.15f);
            m_bacterShader = GetRenderer().CompileShaderProgram(g_bacterShader.m_vertexShader,
                                                              g_bacterShader.m_fragmentShader);

            m_bacters.Init(GetAllocator());
            m_bacters.Obtain()->SetVelocity(m_random.GetReal(-1.0, 1.0) * 20.0f, m_random.GetReal(-1.0, 1.0) * 20.0f);
            m_bacters.Obtain()->SetPosition(58.0f, 0.0f);
            return true;
        }

        ~BacteroidsState()
        {
            GetRenderer().GetGraphics()->DestroyShaderProgram(m_bacterShader);
        }

        void OnResize(int w, int h) override
        {
            int x0 = -w / 2;
            int x1 = w / 2;
            int y0 = -h / 2;
            int y1 = h / 2;
            GetRenderer().SetProjection(Projection_Orthogonal_lh(x0, x1, y0, y1, -1, 1));
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

            for (size_t i = 0; i < m_bacters.size; i++)
            {
                m_bacters[i]->Update(gameTime);
            }
        }

        void Render(const GameTime &gameTime) override
        {
            Renderer &renderer = GetRenderer();
            renderer.SetTime(m_time.GetTime());

            for (size_t i = 0; i < m_bacters.size; i++)
            {
                renderer.BindShader(m_bacterShader);
                m_bacters[i]->Render(&renderer);
            }
        }
    private:
        MicroTicker m_ticker;
        VirtualTime m_time;
        Random m_random;

        ShaderProgramHandle m_bacterShader;

        BacterArray m_bacters;
    };

    bool Bacteroids::Initialize()
    {
        ChangeState<BacteroidsState>();
        return true;
    }

} // bact
