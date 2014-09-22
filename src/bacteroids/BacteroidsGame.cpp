
#include "BacteroidsGame.h"

#include "Player.h"
#include "Bacter.h"
#include "BacterArray.h"

#include "../application/Window.h"
#include "../application/GameState.h"
#include "../time/MicroTicker.h"
#include "../time/VirtualTime.h"
#include "../time/Time.h"

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
            : m_random()
            , m_playerShader(InvalidHandle)
            , m_bacterShader(InvalidHandle)
            , m_fontShader(InvalidHandle)
            , m_textInput()
        {
            m_random.Seed(GetTicks());
        }

        bool Initialize() override
        {
            Renderer &renderer = GetRenderer();

            renderer.SetProjection(Projection_Orthogonal_lh(PLAY_AREA_LEFT,
                                                                 PLAY_AREA_RIGHT,
                                                                 PLAY_AREA_BOTTOM,
                                                                 PLAY_AREA_TOP, -1, 1));

            m_playerShader = renderer.CompileShaderProgram(g_playerShader.m_vertexShader,
                                                           g_playerShader.m_fragmentShader);
            m_bacterShader = renderer.CompileShaderProgram(g_bacterShader.m_vertexShader,
                                                           g_bacterShader.m_fragmentShader);
            m_fontShader = renderer.CompileShaderProgram(g_fontShader.m_vertexShader,
                                                           g_fontShader.m_fragmentShader);

            m_uniforms.velocity = renderer.GetGraphics()->CreateUniform("u_velocity", UniformType::Vec4);
            m_uniforms.anim = renderer.GetGraphics()->CreateUniform("u_anim", UniformType::Float);
            renderer.GetGraphics()->AddProgramUniform(m_playerShader, m_uniforms.velocity);
            renderer.GetGraphics()->AddProgramUniform(m_bacterShader, m_uniforms.anim);

            m_player.SetPosition(0.0f, 0.0f);
            m_playerTarget.SetPosition(0.0f, 0.0f);
            m_bacters.Init(GetAllocator());
            for (int i = 0; i < 6; i++)
            {
                SpawnBacter();
            }
            return true;
        }

        ~BacteroidsState()
        {
            GetRenderer().GetGraphics()->DestroyShaderProgram(m_playerShader);
            GetRenderer().GetGraphics()->DestroyShaderProgram(m_bacterShader);
            GetRenderer().GetGraphics()->DestroyShaderProgram(m_fontShader);
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

        void TogglePause()
        {
            if (m_time.IsPaused())
                m_time.Resume();
            else
                m_time.Pause();
        }

        void OnKeyPress(Keyboard::Key key, Keyboard::Scancode scancode, uint32_t mods) override
        {
            if (key == Keyboard::Key::Escape)
                QuitState();
//            if (key == Key::P)
//                TogglePause();
        }

        void OnKeyDown(Keyboard::Key key, Keyboard::Scancode scancode, uint32_t mods) override
        {
//            TextInputHandler(m_textInput).OnKeyDown(key, mods);
            m_input.SetKey(scancode, true);
        }

        void OnKeyUp(Keyboard::Key key, Keyboard::Scancode scancode, uint32_t mods) override
        { m_input.SetKey(scancode, false); }

        void OnTextInput(const char *str) override
        {
//            TextInputHandler(m_textInput).OnTextInput(str);
        }


        void SpawnBacter()
        {
            Bacter *bacter = m_bacters.Obtain();
            bacter->SetPosition(m_random.GetDirection() * 8.0f);
            bacter->SetTarget(&m_playerTarget);
            bacter->SetAnim(m_random.GetReal(0.0f, 2.0f*PI_f));
        }

        void Update(const GameTime &gameTime) override
        {
            int mx, my, dx, dy;
            const uint32_t buttons = ::SDL_GetMouseState(&mx, &my);
            ::SDL_GetRelativeMouseState(&dx, &dy);
            m_input.SetMouse(mx, my, dx, dy);
            m_input.SetButtons(buttons & SDL_BUTTON_LMASK,
                               buttons & SDL_BUTTON_RMASK,
                               buttons & SDL_BUTTON_MMASK);

            m_player.Update(gameTime, m_input);

            size_t num_bacters[4] = {0};
            Bacter *bacters[4][MAX_BACTERS];

            for (size_t i = 0; i < m_bacters.size; i++)
            {
                Bacter *bacter = m_bacters[i];
                vec4f p = bacter->GetPosition();
                float r = bacter->GetRadius();
                if (p.x <= r && p.y <= r)
                    bacters[0][num_bacters[0]++] = bacter;
                if (p.x <= r && p.y >= -r)
                    bacters[1][num_bacters[1]++] = bacter;
                if (p.x >= -r && p.y <= r)
                    bacters[2][num_bacters[2]++] = bacter;
                if (p.x >= -r && p.y >= -r)
                    bacters[3][num_bacters[3]++] = bacter;
            }

            int n = 0;

            for (size_t i = 0; i < 4; i++)
            {
                size_t num = num_bacters[i];
                Bacter **b = bacters[i];
                for (size_t j = 0; j < num; j++)
                {
                    for (size_t k = j + 1; k < num; k++)
                    {
                        b[j]->DoCollision(b[k]);
                        n++;
                    }
                }
            }

//            int n = 0;

            for (size_t i = 0; i < m_bacters.size; i++)
            {
//                for (size_t j = i + 1; j < m_bacters.size; j++)
//                {
//                    m_bacters[i]->DoCollision(m_bacters[j]);
//                    n++;
//                }

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

//            log::Info("Bacter collision tests: ", n);
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
                Bacter *bacter = m_bacters[i];
                vec4f p = bacter->GetPosition();
                float r = bacter->GetRadius() * 1.5f;

                if (p.x > PLAY_AREA_LEFT - r &&
                    p.x < PLAY_AREA_RIGHT + r &&
                    p.y > PLAY_AREA_BOTTOM - r &&
                    p.y < PLAY_AREA_TOP + r)
                {
                    renderer.BindShader(m_bacterShader);
                    bacter->Render(&renderer, m_uniforms.anim);
                }
            }

//            renderer.BindColorShader();
            renderer.GetGraphics()->SetUniform(m_uniforms.velocity, m_player.GetVelocity());
            renderer.BindShader(m_playerShader);
            m_player.Render(&renderer, m_fontShader);

            SetViewport(m_screenVp);
            const float w = m_screenVp.w;
            const float h = m_screenVp.h;
            renderer.SetProjection(Projection_Orthogonal_lh(0, w, h, 0.0f, -1, 1));

            renderer.SetColor(Color(1.0f, 1.0f, 1.0f));
            renderer.BindFontShader();
            renderer.DrawText(0, 0, "Hello! This is bacteroids");

//            const float cy = 40.0f;
//            renderer.DrawText(0.0f, cy, m_textInput.GetText());
//            const float cx = renderer.GetTextWidth(m_textInput.GetText(), m_textInput.GetCursor());
//            const float cw = 1.0f;
//            const float ch = renderer.GetFontHeight();
//
//            const float cy2 = 70.0f;
//            renderer.DrawTextAscii(0.0f, cy2, m_textInput.GetText());
//            const float cx2 = renderer.GetTextWidthAscii(m_textInput.GetText(), m_textInput.GetCursor());
//            const float cw2 = 1.0f;
//            const float ch2 = renderer.GetFontHeight();
//
//            renderer.BindColorShader();
//            renderer.DrawRectangle(cx, cy, cx + cw, cy + ch);
//            renderer.DrawRectangle(cx2, cy2, cx2 + cw2, cy2 + ch2);
        }

    private:
        Random m_random;

        ShaderProgramHandle m_playerShader;
        ShaderProgramHandle m_bacterShader;
        ShaderProgramHandle m_fontShader;
        struct
        {
            UniformHandle velocity;
            UniformHandle anim;
        } m_uniforms;

        Input m_input;

        Player m_player;
        Target m_playerTarget;
        BacterArray m_bacters;

        Viewport m_playAreaVp;
        Viewport m_screenVp;

        TextInput m_textInput;
    };

    bool Bacteroids::Initialize()
    {
        m_window->SetTitle(
        #if defined(DEBUG_)
            "Bacteroids - Debug"
        #else
            "Bacteroids"
        #endif // DEBUG_
        );
        m_window->GrabMouse();
        ChangeState<BacteroidsState>();
        return true;
    }

    void Bacteroids::OnKeyPress(Keyboard::Key key, Keyboard::Scancode scancode, uint32_t mods)
    {
        if (key == Keyboard::Key::LAlt)
            m_window->ToggleGrabMouse();

        Game::OnKeyPress(key, scancode, mods);
    }

} // bact
