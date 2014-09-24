
#include "BacteroidsGame.h"

#include "Bacter.h"
#include "BacterArray.h"
#include "Projectile.h"
#include "Player.h"
#include "Uniforms.h"

#include "../application/Window.h"
#include "../application/GameState.h"
#include "../time/MicroTicker.h"
#include "../time/VirtualTime.h"
#include "../time/Time.h"

#include "../renderer/Renderer.h"
#include "../graphics/Graphics.h"
#include "../graphics/Texture.h"

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

    const float PLAY_AREA_W         = 24.0f;
    const float PLAY_AREA_H         = PLAY_AREA_W * 0.75f;
    const float PLAY_AREA_LEFT      = -PLAY_AREA_W / 2.0f;
    const float PLAY_AREA_RIGHT     = -PLAY_AREA_LEFT;
    const float PLAY_AREA_BOTTOM    = -PLAY_AREA_H / 2.0f;
    const float PLAY_AREA_TOP       = -PLAY_AREA_BOTTOM;

    class Fade
    {
    public:
        explicit Fade(const Color &color)
            : m_color(color)
            , m_fade(0.0f)
            , m_deltaFade(0.0f)
            , m_maxFade(0.5f)
        { }

        void Activate(float delta)
        { m_deltaFade = delta; }

        void Reset()
        {
            m_deltaFade = 0.0f;
            m_fade = 0.0f;
        }

        void Update(const float deltaTime)
        {
            m_fade += m_deltaFade * deltaTime;
            m_fade = Clamp(m_fade, 0.0f, m_maxFade);
        }

        void Render(Renderer *renderer)
        {
            if (m_fade < 0.01f) return;
            Color c = m_color;
            c.a = m_fade;
            renderer->SetColor(c);
            renderer->DrawFilledRectangle(PLAY_AREA_LEFT, PLAY_AREA_BOTTOM,
                                          PLAY_AREA_RIGHT, PLAY_AREA_TOP);
        }
    private:
        Color m_color;
        float m_fade;
        float m_deltaFade;
        float m_maxFade;
    };

    class BacteroidsState : public GameState
    {
    public:
        BacteroidsState()
            : m_random()
            , m_fade(Color(0.8f, 0.05f, 0.05f))
            , m_pauseFade(Color(0.02f, 0.05f, 0.025f))
            , m_playerShader(InvalidHandle)
            , m_bacterShader(InvalidHandle)
            , m_fontShader(InvalidHandle)
            , m_textInput()
        { }

        bool Initialize() override
        {
//            _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
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

            m_uniforms.m_velocity = renderer.GetGraphics()->CreateUniform("u_velocity", UniformType::Vec4);
            m_uniforms.m_anim = renderer.GetGraphics()->CreateUniform("u_anim", UniformType::Float);
            renderer.GetGraphics()->AddProgramUniform(m_playerShader, m_uniforms.m_velocity);
            renderer.GetGraphics()->AddProgramUniform(m_bacterShader, m_uniforms.m_anim);

            m_player.SetPosition(0.0f, 0.0f);
            m_bacters.Init(GetAllocator());
            m_projectiles.Init(GetAllocator());
            m_objects = GetAllocator().AllocateArray<GameObject*>(MAX_BACTERS + MAX_PROJECTILES + 1);

            m_score = 0;

            m_player.shootSound = GetAudio().LoadSound("data/Laser_Shoot6.wav");
//            m_player.shootSound = GetAudio().LoadSound("data/Hit_Hurt4.wav");
//            m_player.shootSound = GetAudio().LoadSound("data/Explosion5.wav");

            return true;
        }

        ~BacteroidsState()
        {
            GetRenderer().GetGraphics()->DestroyShaderProgram(m_playerShader);
            GetRenderer().GetGraphics()->DestroyShaderProgram(m_bacterShader);
            GetRenderer().GetGraphics()->DestroyShaderProgram(m_fontShader);

            GetAudio().UnloadSound(m_player.shootSound);
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
            {
                m_time.Resume();
                m_pauseFade.Reset();
            }
            else
            {
                m_time.Pause();
                m_pauseFade.Activate(1.0f);
            }
        }

        void OnKeyPress(Keyboard::Key key, Keyboard::Scancode scancode, uint32_t mods) override
        {
            if (key == Keyboard::Key::Escape)
                QuitState();
            if (key == Keyboard::Key::P)
                TogglePause();
            if (key == Keyboard::Key::K)
                m_fade.Activate(1.0f);
            if (key == Keyboard::Key::L)
                m_fade.Reset();
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
            const float D = vec2f(PLAY_AREA_RIGHT, PLAY_AREA_TOP).Length() + 1.5f;

            if (m_bacters.size == MAX_BACTERS) return;

            Bacter *bacter = m_bacters.Obtain();
            bacter->Setup(&m_player, m_random);
            bacter->SetPosition(m_random.GetDirection() * D);
        }

        static Bacter *bacterBuckets[4][MAX_BACTERS];

        void ResolveCollisionsBucketed()
        {
            size_t num_bacters[4] = {0};
            Bacter *(&bacters)[4][MAX_BACTERS] = bacterBuckets;

            for (size_t i = 0; i < m_bacters.size; i++)
            {
                Bacter *bacter = m_bacters[i];
                const vec2f p = bacter->GetPosition();
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
                    b[j]->DoCollision(&m_player);
                    for (size_t k = j + 1; k < num; k++)
                    {
                        b[j]->DoCollision(b[k]);
                        n++;
                    }
                }
            }
//            log::Info("Bacter collision tests: ", n);
        }

        void ResolveCollisions()
        {
            int n = 0;
            for (size_t i = 0; i < m_bacters.size; i++)
            {
                m_bacters[i]->DoCollision(&m_player);
                for (size_t j = i + 1; j < m_bacters.size; j++)
                {
                    m_bacters[i]->DoCollision(m_bacters[j]);
                    n++;
                }
            }
//            log::Info("Bacter collision tests: ", n);
        }

        void DoCollisions()
        {
            size_t num_objects = 0;
            for (size_t i = 0; i < m_bacters.size; i++)
                m_objects[num_objects++] = m_bacters[i];
            for (size_t i = 0; i < m_projectiles.size; i++)
                m_objects[num_objects++] = m_projectiles[i];
            m_objects[num_objects++] = &m_player;

            for (size_t i = 0; i < num_objects; i++)
            {
                GameObject *obj_i = m_objects[i];
                vec2f pi = obj_i->GetPosition();
                float ri = obj_i->GetRadius();

                for (size_t j = i + 1; j < num_objects; j++)
                {
                    GameObject *obj_j = m_objects[j];
                    vec2f pj = obj_j->GetPosition();
                    float rj = obj_j->GetRadius();

                    vec2f j_to_i = (pi - pj);
                    float d = ri + rj - j_to_i.Length();

                    if (d > 0.0f)
                    {
                        obj_i->DoCollision2(obj_j, j_to_i, d);
                        obj_j->DoCollision2(obj_i, -j_to_i, d);
                    }
                }
            }
        }

        void UpdatePlayer(const GameTime &gameTime)
        {
            int mx, my, dx, dy;
            const uint32_t buttons = ::SDL_GetMouseState(&mx, &my);
            ::SDL_GetRelativeMouseState(&dx, &dy);
            m_input.SetMouse(mx, my, dx, dy);
            m_input.SetButtons(buttons & SDL_BUTTON_LMASK,
                               buttons & SDL_BUTTON_RMASK,
                               buttons & SDL_BUTTON_MMASK);

            m_player.Update(gameTime, m_input, m_projectiles, GetAudio());

            vec2f pl_pos = m_player.GetPosition();
            const float pl_radius = m_player.GetRadius();

                 if (pl_pos.x < PLAY_AREA_LEFT + pl_radius)   pl_pos.x = PLAY_AREA_LEFT + pl_radius;
            else if (pl_pos.x > PLAY_AREA_RIGHT - pl_radius)  pl_pos.x = PLAY_AREA_RIGHT - pl_radius;
                 if (pl_pos.y < PLAY_AREA_BOTTOM + pl_radius) pl_pos.y = PLAY_AREA_BOTTOM + pl_radius;
            else if (pl_pos.y > PLAY_AREA_TOP - pl_radius)    pl_pos.y = PLAY_AREA_TOP - pl_radius;

            m_player.SetPosition(pl_pos);
        }

        void RealtimeUpdate(const Time_t deltaMicroseconds) override
        {
            const float deltaTime = float(deltaMicroseconds) / 1e6f;
            m_pauseFade.Update(deltaTime);
        }

        void Update(const GameTime &gameTime) override
        {
            static float spawn_rate = 0.1f;
            static float num_spawn = 0.0f;
            spawn_rate = std::log(1.0f + gameTime.GetTotalSeconds() * 0.1f) * 0.5f;
            num_spawn += spawn_rate * gameTime.GetDeltaSeconds();
            while (num_spawn >= 1.0f)
            {
                SpawnBacter();
                num_spawn -= 1.0f;
            }

            UpdatePlayer(gameTime);

            DoCollisions();

//            ResolveCollisionsBucketed();

            for (size_t i = 0; i < m_bacters.size; i++)
            {
                m_bacters[i]->Update(gameTime);
                Bacter::TrySplit(m_bacters[i], m_bacters, m_random);
            }

            size_t m_bacters_size = m_bacters.size;

            for (size_t i = 0; i < m_projectiles.size; i++)
            {
                for (size_t b = 0; b < m_bacters_size; b++)
                {
                    if (m_projectiles[i]->DoCollision(m_bacters[b]))
                    {
                        int points = m_bacters[b]->TakeHit(m_bacters, m_random);
                        m_score += points;
                    }
                }

                m_projectiles[i]->Update(gameTime);
            }

            for (size_t i = 0; i < m_projectiles.size; )
            {
                if (!m_projectiles[i]->IsAlive())
                {
                    m_projectiles.Remove(i);
                    continue;
                }
                i++;
            }

            for (size_t i = 0; i < m_bacters.size; )
            {
                if (!m_bacters[i]->IsAlive())
                {
                    m_bacters.Remove(i);
                    continue;
                }
                i++;
            }

            m_fade.Update(gameTime.GetDeltaSeconds());
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

            renderer.BindShader(m_bacterShader);
            for (size_t i = 0; i < m_bacters.size; i++)
            {
                Bacter *bacter = m_bacters[i];
                vec2f p = bacter->GetPosition();
                float r = bacter->GetRadius() * 1.5f;

                if (p.x > PLAY_AREA_LEFT - r &&
                    p.x < PLAY_AREA_RIGHT + r &&
                    p.y > PLAY_AREA_BOTTOM - r &&
                    p.y < PLAY_AREA_TOP + r)
                {
                    renderer.BindShader(m_bacterShader);
                    bacter->Render(&renderer, m_uniforms);
                }
            }

            renderer.BindShader(m_playerShader);
            m_player.Render(&renderer, m_uniforms);

            renderer.BindColorShader();
            for (size_t i = 0; i < m_projectiles.size; i++)
            {
                m_projectiles[i]->Render(&renderer, m_uniforms);
            }

            m_fade.Render(&renderer);
            m_pauseFade.Render(&renderer);

            SetViewport(m_screenVp);
            const float w = m_screenVp.w;
            const float h = m_screenVp.h;
            renderer.SetProjection(Projection_Orthogonal_lh(0, w, h, 0.0f, -1, 1));

            char buf[64];
            StringPrintF(buf, "Score: %i", m_score);

            renderer.SetColor(Color(1.0f, 1.0f, 1.0f));
            renderer.BindFontShader();
            renderer.DrawText(0, 0, buf);
//            renderer.DrawText(0, 0, "Hello! This is bacteroids");

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
        BacteroidsUniforms m_uniforms;

        Input m_input;

        Player m_player;
        BacterArray m_bacters;
        ProjectileArray m_projectiles;

        Fade m_fade;
        Fade m_pauseFade;

        GameObject **m_objects;

        int m_score;

        Viewport m_playAreaVp;
        Viewport m_screenVp;

        TextInput m_textInput;
    };

    Bacter *BacteroidsState::bacterBuckets[4][MAX_BACTERS];


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
        if (key == Keyboard::Key::Num0)
        {
            Texture *tex = m_renderer->GetFontTexture();
            log::Debug("Font texture: ", tex->GetObject(), " ", tex->GetWidth(), " ", tex->GetHeight());
        }

        Game::OnKeyPress(key, scancode, mods);
    }

} // bact
