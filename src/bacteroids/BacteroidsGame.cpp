
#include "BacteroidsGame.h"

#include "Player.h"
#include "ObjectArray.h"
#include "Uniforms.h"
#include "SoundPlayer.h"

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
            , m_playerShader(InvalidHandle)
            , m_bacterShader(InvalidHandle)
            , m_fontShader(InvalidHandle)
            , m_damageFade(Color(0.8f, 0.05f, 0.05f))
            , m_pauseFade(Color(0.02f, 0.05f, 0.025f))
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

            m_soundPlayer.Init(GetAudio(), GetCache());

            m_player.SetPosition(0.0f, 0.0f);
            m_objects.Init(GetAllocator());
            m_objects.AddObject(&m_player);

            m_score = 0;

            for (size_t i = 0; i < 6; i++)
                SpawnBacter(0.5f);

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
            const float x_scl = w / PLAY_AREA_W;
            const float y_scl = h / PLAY_AREA_H;
            const float scale = (x_scl < y_scl) ? x_scl : y_scl;

            const int vpW = scale * PLAY_AREA_W;
            const int vpH = scale * PLAY_AREA_H;
            m_playView.SetViewport((w - vpW) / 2, (h - vpH) / 2, vpW, vpH);
            m_playView.m_projection = Projection_Orthogonal_lh(PLAY_AREA_LEFT,
                                                               PLAY_AREA_RIGHT,
                                                               PLAY_AREA_BOTTOM,
                                                               PLAY_AREA_TOP, -1.0f, 1.0f);
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
            if (key == Keyboard::Key::M)
                GetAudio().ToggleMute();
            if (key == Keyboard::Key::K)
                m_damageFade.Activate(1.0f);
            if (key == Keyboard::Key::L)
                m_damageFade.Reset();
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


        void SpawnBacter(float distMod = 1.0f)
        {
            const float D = vec2f(PLAY_AREA_RIGHT, PLAY_AREA_TOP).Length() * distMod + 1.5f;

            if (!m_objects.CanObtainBacter()) return;

            Bacter *bacter = m_objects.ObtainBacter();
            bacter->Setup(&m_player, m_random);
            bacter->SetPosition(m_random.GetDirection() * D);
        }

//        static Bacter *bacterBuckets[4][MAX_BACTERS];

//        void ResolveCollisionsBucketed()
//        {
//            size_t num_bacters[4] = {0};
//            Bacter *(&bacters)[4][MAX_BACTERS] = bacterBuckets;
//
//            for (size_t i = 0; i < m_bacters.size; i++)
//            {
//                Bacter *bacter = m_bacters[i];
//                const vec2f p = bacter->GetPosition();
//                float r = bacter->GetRadius();
//                if (p.x <= r && p.y <= r)
//                    bacters[0][num_bacters[0]++] = bacter;
//                if (p.x <= r && p.y >= -r)
//                    bacters[1][num_bacters[1]++] = bacter;
//                if (p.x >= -r && p.y <= r)
//                    bacters[2][num_bacters[2]++] = bacter;
//                if (p.x >= -r && p.y >= -r)
//                    bacters[3][num_bacters[3]++] = bacter;
//            }
//
//            int n = 0;
//            for (size_t i = 0; i < 4; i++)
//            {
//                size_t num = num_bacters[i];
//                Bacter **b = bacters[i];
//                for (size_t j = 0; j < num; j++)
//                {
//                    b[j]->DoCollision(&m_player);
//                    for (size_t k = j + 1; k < num; k++)
//                    {
//                        b[j]->DoCollision(b[k]);
//                        n++;
//                    }
//                }
//            }
////            log::Info("Bacter collision tests: ", n);
//        }

        void BacterCollision(Bacter *me, GameObject *obj, const vec2f &objToMe, float dist)
        {
            if (obj->GetType() == Bacter::TYPE || obj->GetType() == Player::TYPE)
            {
                vec2f v = objToMe.SafeNormalized() * dist/8.0f;
                vec2f p = me->GetPosition();
                me->AddVelocity(v);
                me->SetPosition(p + (v / 2.0f));
                float r = me->GetRadius() + obj->GetRadius();
                me->ModifySize(1.0f + dist / r);
            }
            else if (obj->GetType() == Projectile::TYPE)
            {
                me->AddVelocity(obj->GetVelocity() * 0.5f);
                m_score += me->TakeHit(m_objects, m_random);
            }
        }

        void PlayerCollision(Player *me, GameObject *obj, const vec2f &objToMe, float dist)
        {
            if (obj->GetType() == Bacter::TYPE)
            {
                vec2f v = objToMe.SafeNormalized() * dist/8.0f;
                vec2f p = me->GetPosition();
                me->AddVelocity(v);
                me->SetPosition(p + (v / 2.0f));
                me->TakeHit();
            }
        }

        void ProjectileCollision(Projectile *me, GameObject *obj, const vec2f &objToMe, float dist)
        {
            if (obj->GetType() == Bacter::TYPE)
            {
                me->SetAlive(false);
            }
        }

        void DoCollision(GameObject *obj1, GameObject *obj2, const vec2f &from2To1, float dist)
        {
            switch (obj1->GetType())
            {
            case Bacter::TYPE:
                BacterCollision((Bacter*)obj1, obj2, from2To1, dist);
                break;

            case Player::TYPE:
                PlayerCollision((Player*)obj1, obj2, from2To1, dist);
                break;

            case Projectile::TYPE:
                ProjectileCollision((Projectile*)obj1, obj2, from2To1, dist);
                break;

            default:
                break;
            }
        }

        void DoCollisions()
        {
            size_t num_objects = m_objects.Size();

            for (size_t i = 0; i < num_objects; i++)
            {
                GameObject *obj1 = m_objects[i];
                vec2f p1 = obj1->GetPosition();
                float r1 = obj1->GetRadius();

                for (size_t j = i + 1; j < num_objects; j++)
                {
                    GameObject *obj2 = m_objects[j];
                    vec2f p2 = obj2->GetPosition();
                    float r2 = obj2->GetRadius();

                    vec2f from2To1 = (p1 - p2);
                    float dist = r1 + r2 - from2To1.Length();

                    if (dist > 0.0f)
                    {
                        DoCollision(obj1, obj2, from2To1, dist);
                        DoCollision(obj2, obj1, -from2To1, dist);
                    }
                }
            }
        }

        void UpdatePlayer(const GameTime &gameTime)
        {
            if (!m_player.IsAlive()) return;

            m_input.UpdateMouse();
            m_player.Update(gameTime, m_input, m_objects, m_soundPlayer);

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
            static float num_spawn = 1.0f;
            spawn_rate = std::log(10.0f + gameTime.GetTotalSeconds() * 0.1f) * 0.5f;
            num_spawn += spawn_rate * gameTime.GetDeltaSeconds();
            while (num_spawn >= 1.0f)
            {
                SpawnBacter();
                num_spawn -= 1.0f;
            }

            UpdatePlayer(gameTime);

            DoCollisions();

            for (size_t i = 0; i < m_objects.Size(); i++)
            {
                m_objects[i]->Update(gameTime);
            }

            for (size_t i = 0; i < m_objects.Size(); )
            {
                if (!m_objects[i]->IsAlive())
                {
                    m_objects.Remove(i);
                    continue;
                }
                i++;
            }

//            for (size_t i = 0; i < m_bacters.size; i++)
//            {
//                m_bacters[i]->Update(gameTime);
//                Bacter::TrySplit(m_bacters[i], m_bacters, m_random); // TODO: spontaneous bacter splitting needed!
//            }

            m_damageFade.Update(gameTime.GetDeltaSeconds());
        }

        void Render() override
        {
            Renderer &renderer = GetRenderer();

            renderer.SetView(m_playView);

            renderer.BindColorShader();
            renderer.SetColor(Color(0.05f, 0.13f, 0.15f));
            renderer.DrawFilledRectangle(PLAY_AREA_LEFT, PLAY_AREA_BOTTOM, PLAY_AREA_RIGHT, PLAY_AREA_TOP);
            renderer.SetTime(m_time.GetTimeMicros());

            for (size_t i = 0; i < m_objects.Size(); i++)
            {
                GameObject *obj = m_objects[i];

                vec2f p = obj->GetPosition();
                float r = obj->GetRadius() * 1.5f;

                if ((p.x > PLAY_AREA_LEFT - r && p.x < PLAY_AREA_RIGHT + r &&
                     p.y > PLAY_AREA_BOTTOM - r && p.y < PLAY_AREA_TOP + r) == false)
                {
                    continue;
                }

                switch (obj->GetType())
                {
                case Bacter::TYPE:
                    renderer.BindShader(m_bacterShader);
                    break;

                case Player::TYPE:
                    renderer.BindShader(m_playerShader);
                    break;

                case Projectile::TYPE:
                    renderer.BindColorShader();
                    break;

                default:
                    ROB_ASSERT(0);
                    break;
                }

                obj->Render(&renderer, m_uniforms);
            }

            renderer.BindColorShader();
            m_damageFade.Render(&renderer);
            m_pauseFade.Render(&renderer);

            renderer.SetView(GetDefaultView());
            renderer.SetFontScale(1.0f);
            char buf[64];
            StringPrintF(buf, "Score: %i", m_score);

            renderer.SetColor(Color(1.0f, 1.0f, 1.0f));
            renderer.BindFontShader();
            renderer.DrawText(0, 0, buf);

            const float hx = 10.0f;
            const float hy = 10.0f + renderer.GetFontHeight();

            renderer.BindColorShader();
            renderer.SetColor(Color(0.5f, 0.5f, 0.5f));
            renderer.DrawFilledRectangle(hx, hy, hx + 100.0f, hy + 10.0f);
            renderer.SetColor(Color(0.85f, 0.05f, 0.05f));
            renderer.DrawFilledRectangle(hx, hy, hx + m_player.GetHealth(), hy + 10.0f);

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
        SoundPlayer m_soundPlayer;

        Input m_input;

        Player m_player;
        ObjectArray m_objects;

        int m_score;

        Fade m_damageFade;
        Fade m_pauseFade;

        View m_playView;

        TextInput m_textInput;
    };

//    Bacter *BacteroidsState::bacterBuckets[4][MAX_BACTERS];


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
