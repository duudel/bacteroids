
#include "BacteroidsGame.h"

#include "Bacter.h"
#include "BacterArray.h"

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

#include <SDL2/SDL.h>

namespace bact
{

    using namespace rob;

    struct Viewport
    {
        int x, y, w, h;
    };

    const float PLAY_AREA_W         = 20.0f;
    const float PLAY_AREA_H         = 15.0f;
    const float PLAY_AREA_LEFT      = -PLAY_AREA_W / 2.0f;
    const float PLAY_AREA_RIGHT     = -PLAY_AREA_LEFT;
    const float PLAY_AREA_BOTTOM    = -PLAY_AREA_H / 2.0f;
    const float PLAY_AREA_TOP       = -PLAY_AREA_BOTTOM;

    class BacteroidsState : public GameState
    {
    public:
        BacteroidsState()
        {
            m_random.Seed(SDL_GetTicks());
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
            const float DIST = 8.0f;

            Bacter *bacter = m_bacters.Obtain();
            float r = m_random.GetReal(0.0f, 2.0f*PI_f);
            bacter->SetPosition(Cos(r)*DIST, Sin(r)*DIST);
            bacter->SetTarget(&m_player);
            bacter->SetAnim(m_random.GetReal(0.0f, 2.0f*PI_f));
        }

        void Update(const GameTime &gameTime) override
        {
            m_time.Update();

            for (size_t i = 0; i < m_bacters.size; i++)
            {
                for (size_t j = i+1; j < m_bacters.size; j++)
                    m_bacters[i]->DoCollision(m_bacters[j]);

                m_bacters[i]->Update(gameTime);

                if (m_bacters[i]->ShouldClone())
                {
                    if (m_bacters.size < MAX_BACTERS)
                    {
                        Bacter *b = m_bacters.Obtain();
                        m_bacters[i]->Clone(b, m_random);
                    }
                }
            }
        }

        void SetViewport(Viewport &vp)
        {
            GetRenderer().GetGraphics()->SetViewport(vp.x, vp.y, vp.w, vp.h);
        }

        void Render() override
        {
            Renderer &renderer = GetRenderer();

            SetViewport(m_playAreaVp);
            renderer.SetProjection(Projection_Orthogonal_lh(PLAY_AREA_LEFT,
                                                            PLAY_AREA_RIGHT,
                                                            PLAY_AREA_BOTTOM,
                                                            PLAY_AREA_TOP, -1, 1));

            renderer.BindColorShader();
            renderer.SetColor(Color(0.05f, 0.13f, 0.15f));
            renderer.DrawFilledRectangle(PLAY_AREA_LEFT, PLAY_AREA_BOTTOM, PLAY_AREA_RIGHT, PLAY_AREA_TOP);
            renderer.SetTime(m_time.GetTime());

            for (size_t i = 0; i < m_bacters.size; i++)
            {
                renderer.BindShader(m_bacterShader);
                m_bacters[i]->Render(&renderer, m_uniforms.anim);
            }

            SetViewport(m_screenVp);
            float aspect = m_screenVp.w / m_screenVp.h;
            float h2 = m_screenVp.h;// / 2.0f;
            float w2 = h2 * aspect;
            renderer.SetProjection(Projection_Orthogonal_lh(0,
                                                            w2,
                                                            h2,
                                                            0.0f, -1, 1));

            renderer.SetColor(Color(1.0f, 1.0f, 1.0f));
            renderer.BindFontShader();
            renderer.DrawText(0, 0, "Heei maailma! This is bacteroids");
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
