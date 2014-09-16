
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

    class Target
    {
    public:
        void SetPosition(float x, float y)
        { m_position = vec4f(x, y, 0.0f, 1.0f); }

        vec4f GetPosition() const
        { return m_position; }

    private:
        vec4f m_position;
    };

    vec4f Normalize(const vec4f &v)
    {
        float len = v.Length();
        return (len > 0.1f) ? v/len : v;
    }

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
        void SetAnim(float anim)
        { m_anim = anim; }
        void SetTarget(Target *target)
        { m_target = target; }

        vec4f GetPosition() const
        { return m_position; }
        float GetRadius() const
        { return m_radius; }

        void AddVelocity(const vec4f &v)
        { m_velocity += v; }

        void Update(const GameTime &gameTime)
        {
            const float MAX_V = 50.0f;
            const float ACCEL = 50.0f;
            const float FRICT = 0.05f;
            const float GROWTH = 5.0f;

            const float dt = gameTime.GetDeltaSeconds();

            if (m_target)
            {
                vec4f d = Normalize(m_target->GetPosition() - m_position);
                vec4f v = m_velocity + d * ACCEL * dt;
                if (v.Length() > MAX_V && m_velocity.Length() < v.Length()) ;
                else m_velocity = v;
            }

            m_position += m_velocity * dt;
            m_velocity -= m_velocity * FRICT * dt;

            if (!CanSplit())
                m_radius += GROWTH * dt;
        }

        bool CanSplit()
        {
            const float SPLIT_RADIUS = 55.0f;
            return m_radius > SPLIT_RADIUS;
        }

        void Render(Renderer *renderer, UniformHandle anim)
        {
            renderer->GetGraphics()->SetUniform(anim, m_anim);
            renderer->SetColor(Color(1.0f, 1.2f, 0.6f));
//            renderer->DrawFilledCirlce(m_position.x, m_position.y, m_radius);
            renderer->DrawFilledCirlce(m_position.x, m_position.y, m_radius, Color(0.0f, 0.5f, 0.5f, 1.0f));
        }

    private:
        vec4f m_position;
        vec4f m_velocity;
        float m_radius;
        float m_anim;
        Target *m_target;
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
            Renderer &renderer = GetRenderer();

            renderer.SetProjection(Projection_Orthogonal_lh(PLAY_AREA_LEFT,
                                                                 PLAY_AREA_RIGHT,
                                                                 PLAY_AREA_BOTTOM,
                                                                 PLAY_AREA_TOP, -1, 1));

            m_bacterShader = renderer.CompileShaderProgram(g_bacterShader.m_vertexShader,
                                                           g_bacterShader.m_fragmentShader);

            m_uniforms.anim = renderer.GetGraphics()->CreateUniform("u_anim", UniformType::Float);
            renderer.GetGraphics()->AddProgramUniform(m_bacterShader, m_uniforms.anim);

            m_player.SetPosition(0.0f, 0.0f);
            m_bacters.Init(GetAllocator());
            for (int i = 0; i < 6; i++)
            {
                SpawnBacter();
            }
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

        void SpawnBacter()
        {
            const float DIST = 300.0f;

            Bacter *bacter = m_bacters.Obtain();
            float r = m_random.GetReal(0.0f, 2.0f*PI_f);
            bacter->SetPosition(Cos(r)*DIST, Sin(r)*DIST);
            bacter->SetTarget(&m_player);
            bacter->SetAnim(m_random.GetReal(0.0f, 2.0f*PI_f));
        }

        float Distance(const vec4f &a, const vec4f &b)
        { return (b - a).Length(); }

        void Update(const GameTime &gameTime) override
        {
            m_time.Update();

            int n = 0;
            Bacter *split[100];

            for (size_t i = 0; i < m_bacters.size; i++)
            {
                vec4f a = m_bacters[i]->GetPosition();
                float ra = m_bacters[i]->GetRadius();
                for (size_t j = i+1; j < m_bacters.size; j++)
                {
                    vec4f b = m_bacters[j]->GetPosition();
                    float rb = m_bacters[j]->GetRadius();
                    float d = ra + rb - Distance(a, b);
                    if (d > 0.0f)
                    {
                        vec4f v = Normalize(a - b) * d/8.0f;
//                        vec4f v2 = v/32.0f;
//                        a += v2;
//                        b -= v2;
//                        m_bacters[i]->SetPosition(a.x, a.y);
//                        m_bacters[j]->SetPosition(b.x, b.y);
                        m_bacters[i]->AddVelocity(v);
                        m_bacters[j]->AddVelocity(-v);
                    }
                }

                m_bacters[i]->Update(gameTime);

                if (m_bacters[i]->CanSplit() && n < 100)
                {
                    split[n++] = m_bacters[i];
                }
            }

            for (int i = 0; i < n; i++)
            {
                vec4f p = split[i]->GetPosition();
                float r = split[i]->GetRadius();
                Bacter *bacter = m_bacters.Obtain();
                bacter->SetPosition(p.x, p.y);
                bacter->SetRadius(r/2.0f);
                bacter->SetTarget(&m_player);
                split[i]->SetRadius(r/2.0f);
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
                m_bacters[i]->Render(&renderer, m_uniforms.anim);
            }
        }
    private:
        Random m_random;

        ShaderProgramHandle m_bacterShader;
        struct
        {
            UniformHandle anim;
        } m_uniforms;

        Target m_player;
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
