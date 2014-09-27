
#include "BacteroidsState.h"

#include "Shaders.h"

#include "../graphics/Graphics.h"
#include "../renderer/Renderer.h"
#include "../application/Window.h"

#include "../math/Math.h"
#include "../math/Projection.h"

namespace bact
{

    static const float PLAY_AREA_W      = 24.0f;
    static const float PLAY_AREA_H      = PLAY_AREA_W * 0.75f;
    static const float PLAY_AREA_LEFT   = -PLAY_AREA_W / 2.0f;
    static const float PLAY_AREA_RIGHT  = -PLAY_AREA_LEFT;
    static const float PLAY_AREA_BOTTOM = -PLAY_AREA_H / 2.0f;
    static const float PLAY_AREA_TOP    = -PLAY_AREA_BOTTOM;


    BacteroidsState::BacteroidsState()
        : m_random()
        , m_playerShader(InvalidHandle)
        , m_bacterShader(InvalidHandle)
        , m_projectileShader(InvalidHandle)
        , m_fontShader(InvalidHandle)
        , m_damageFade(Color(0.8f, 0.05f, 0.05f))
        , m_pauseFade(Color(0.02f, 0.05f, 0.025f))
        , m_textInput()
    { }

    bool BacteroidsState::Initialize()
    {
//            _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
        Renderer &renderer = GetRenderer();

        m_playerShader = renderer.CompileShaderProgram(g_playerShader.m_vertexShader,
                                                       g_playerShader.m_fragmentShader);
        m_bacterShader = renderer.CompileShaderProgram(g_bacterShader.m_vertexShader,
                                                       g_bacterShader.m_fragmentShader);
        m_projectileShader = renderer.CompileShaderProgram(g_projectileShader.m_vertexShader,
                                                       g_projectileShader.m_fragmentShader);
        m_fontShader = renderer.CompileShaderProgram(g_fontShader.m_vertexShader,
                                                       g_fontShader.m_fragmentShader);

        m_uniforms.m_velocity = renderer.GetGraphics()->CreateUniform("u_velocity", UniformType::Vec4);
        m_uniforms.m_anim = renderer.GetGraphics()->CreateUniform("u_anim", UniformType::Float);

        renderer.GetGraphics()->AddProgramUniform(m_playerShader, m_uniforms.m_velocity);
        renderer.GetGraphics()->AddProgramUniform(m_projectileShader, m_uniforms.m_velocity);

        renderer.GetGraphics()->AddProgramUniform(m_bacterShader, m_uniforms.m_anim);
        renderer.GetGraphics()->AddProgramUniform(m_bacterShader, m_uniforms.m_velocity);

        m_soundPlayer.Init(GetAudio(), GetCache());

        m_player.SetPosition(0.0f, 0.0f);
        m_objects.Init(GetAllocator());
        m_objects.AddObject(&m_player);

        m_quadTree = GetAllocator().AllocateArray<GameObject*>(MAX_OBJECTS);

        m_score = 0;

        m_damageFade.SetFadeAcceleration(-10.0f);

        for (size_t i = 0; i < 6; i++)
            SpawnBacter(0.5f);

        return true;
    }

    BacteroidsState::~BacteroidsState()
    {
        GetRenderer().GetGraphics()->DestroyShaderProgram(m_playerShader);
        GetRenderer().GetGraphics()->DestroyShaderProgram(m_bacterShader);
        GetRenderer().GetGraphics()->DestroyShaderProgram(m_projectileShader);
        GetRenderer().GetGraphics()->DestroyShaderProgram(m_fontShader);
    }

    void BacteroidsState::OnResize(int w, int h)
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

    void BacteroidsState::TogglePause()
    {
        if (m_time.IsPaused())
        {
            m_time.Resume();
            m_pauseFade.Reset();
            GetWindow().GrabMouse();
        }
        else
        {
            m_time.Pause();
            m_pauseFade.Activate(1.0f);
            GetWindow().UnGrabMouse();
        }
    }

    void BacteroidsState::OnKeyPress(Keyboard::Key key, Keyboard::Scancode scancode, uint32_t mods)
    {
        if (key == Keyboard::Key::Escape)
            ChangeState(1);
        if (key == Keyboard::Key::P)
            TogglePause();
        if (key == Keyboard::Key::M)
            GetAudio().ToggleMute();

        if (m_player.IsDead())
        {
            if (key == Keyboard::Key::Space)
                ChangeState(3);
        }
    }

    void BacteroidsState::OnKeyDown(Keyboard::Key key, Keyboard::Scancode scancode, uint32_t mods)
    {
//            TextInputHandler(m_textInput).OnKeyDown(key, mods);
        m_input.SetKey(scancode, true);
    }

    void BacteroidsState::OnKeyUp(Keyboard::Key key, Keyboard::Scancode scancode, uint32_t mods)
    { m_input.SetKey(scancode, false); }

    void BacteroidsState::OnTextInput(const char *str)
    {
//            TextInputHandler(m_textInput).OnTextInput(str);
    }


    void BacteroidsState::SpawnBacter(float distMod /*= 1.0f*/)
    {
        const float D = vec2f(PLAY_AREA_RIGHT, PLAY_AREA_TOP).Length() * distMod + 1.5f;

        if (!m_objects.CanObtainBacter()) return;

        Bacter *bacter = m_objects.ObtainBacter();
        bacter->Setup(&m_player, m_random);
        bacter->SetPosition(m_random.GetDirection() * D);
    }

    void BacteroidsState::BacterCollision(Bacter *me, GameObject *obj, const vec2f &objToMe, float dist)
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

    void BacteroidsState::PlayerCollision(Player *me, GameObject *obj, const vec2f &objToMe, float dist)
    {
        if (obj->GetType() == Bacter::TYPE)
        {
            vec2f v = objToMe.SafeNormalized() * dist/8.0f;
            vec2f p = me->GetPosition();
            me->AddVelocity(v);
            me->SetPosition(p + (v / 2.0f));
            me->TakeHit();
            m_damageFade.Activate(1.0f);
            if (me->IsDead())
            {
                m_damageFade.SetFadeAcceleration(0.0f);
                m_damageFade.Activate(1.0f);
                m_soundPlayer.PlayPlayerDeathSound();
            }
        }
    }

    void BacteroidsState::ProjectileCollision(Projectile *me, GameObject *obj, const vec2f &objToMe, float dist)
    {
        if (obj->GetType() == Bacter::TYPE)
        {
            me->SetAlive(false);
        }
    }

    void BacteroidsState::DoCollision(GameObject *obj1, GameObject *obj2, const vec2f &from2To1, float dist)
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

    void BacteroidsState::DoCollisions()
    {
    #define BUCKETED_COLLISION 1
    #if defined(BUCKETED_COLLISION)
        size_t num_objects = m_objects.Size();

        size_t q[5] = {0};

        for (size_t i = 0; i < num_objects; i++)
        {
            GameObject *o = m_objects[i];
            vec2f p = o->GetPosition();
            float r = o->GetRadius();

            if (p.x + r < 0.0f && p.y + r < 0.0f)
            {
                size_t q0 = q[0]++;
                size_t q1 = q[1]++;
                size_t q2 = q[2]++;
                size_t q3 = q[3]++;
                size_t q4 = q[4]++;
                m_quadTree[q4] = m_quadTree[q3];
                m_quadTree[q3] = m_quadTree[q2];
                m_quadTree[q2] = m_quadTree[q1];
                m_quadTree[q1] = m_quadTree[q0];
                m_quadTree[q0] = o;
            }
            else if (p.x + r < 0.0f && p.y - r > 0.0f)
            {
                size_t q1 = q[1]++;
                size_t q2 = q[2]++;
                size_t q3 = q[3]++;
                size_t q4 = q[4]++;
                m_quadTree[q4] = m_quadTree[q3];
                m_quadTree[q3] = m_quadTree[q2];
                m_quadTree[q2] = m_quadTree[q1];
                m_quadTree[q1] = o;
            }
            else if (p.x - r > 0.0f && p.y + r < 0.0f)
            {
                size_t q2 = q[2]++;
                size_t q3 = q[3]++;
                size_t q4 = q[4]++;
                m_quadTree[q4] = m_quadTree[q3];
                m_quadTree[q3] = m_quadTree[q2];
                m_quadTree[q2] = o;
            }
            else if (p.x - r > 0.0f && p.y - r > 0.0f)
            {
                size_t q3 = q[3]++;
                size_t q4 = q[4]++;
                m_quadTree[q4] = m_quadTree[q3];
                m_quadTree[q3] = o;
            }
            else
            {
                size_t q4 = q[4]++;
                m_quadTree[q4] = o;
            }
        }

        for (size_t k = 0, i = 0; k < 5; k++)
        {
            for (; i < q[k]; i++)
            {
                GameObject *obj1 = m_quadTree[i];
                vec2f p1 = obj1->GetPosition();
                float r1 = obj1->GetRadius();

                for (size_t j = i + 1; j < q[k]; j++)
                {
                    GameObject *obj2 = m_quadTree[j];
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

                if (k == 4) continue;

                for (size_t j = q[3]; j < q[4]; j++)
                {
                    GameObject *obj2 = m_quadTree[j];
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
    #else
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
    #endif // BUCKETED_COLLISION
    }

    void BacteroidsState::UpdatePlayer(const GameTime &gameTime)
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

    void BacteroidsState::RealtimeUpdate(const Time_t deltaMicroseconds)
    {
        const float deltaTime = float(deltaMicroseconds) / 1e6f;
        m_pauseFade.Update(deltaTime);
    }

    void BacteroidsState::Update(const GameTime &gameTime)
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

    void BacteroidsState::RenderPause()
    {
        Renderer &renderer = GetRenderer();
        renderer.SetFontScale(4.0f);
        renderer.SetColor(Color(1.0f, 1.0f, 1.0f));
        renderer.BindFontShader();

        const Viewport vp = renderer.GetView().m_viewport;
//            const float fontH = renderer.GetFontHeight();
        const float textW = renderer.GetTextWidth("Game paused");
        renderer.DrawText((vp.w - textW) / 2.0f, vp.h / 3.0f, "Game paused");
    }

    void BacteroidsState::RenderGameOver()
    {
        Renderer &renderer = GetRenderer();
        renderer.SetFontScale(4.0f);
        renderer.SetColor(Color(1.0f, 1.0f, 1.0f));
        renderer.BindFontShader();

        const Viewport vp = renderer.GetView().m_viewport;

        const float fontH = renderer.GetFontHeight();
        const char * const gameOver = "Game over";
        const float gameOverW = renderer.GetTextWidth(gameOver);
        renderer.DrawText((vp.w - gameOverW) / 2.0f, vp.h / 3.0f, gameOver);

        renderer.SetFontScale(1.0f);
        const char * const instruction = "Press [space] to continue";
        const float textW = renderer.GetTextWidth(instruction);
        renderer.DrawText((vp.w - textW) / 2.0f, vp.h / 3.0f + fontH, instruction);
    }

    void BacteroidsState::Render()
    {
        Renderer &renderer = GetRenderer();
        renderer.SetTime(m_time.GetTimeMicros());

        renderer.SetView(m_playView);

        renderer.BindColorShader();
        renderer.SetColor(Color(0.05f, 0.13f, 0.15f));
        renderer.DrawFilledRectangle(PLAY_AREA_LEFT, PLAY_AREA_BOTTOM, PLAY_AREA_RIGHT, PLAY_AREA_TOP);

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
                renderer.BindShader(m_projectileShader);
                break;

            default:
                ROB_ASSERT(0);
                break;
            }

            const vec2f vel2 = obj->GetVelocity();
            const vec4f velocity(vel2.x, vel2.y, 0.0f, 0.0f);
            renderer.GetGraphics()->SetUniform(m_uniforms.m_velocity, velocity);
            obj->Render(&renderer, m_uniforms);
        }

        renderer.BindColorShader();
        m_damageFade.Render(&renderer);
        m_pauseFade.Render(&renderer);


        renderer.SetView(GetDefaultView());

        if (m_time.IsPaused())
        {
            RenderPause();
        }
        else if (m_player.IsDead())
        {
            RenderGameOver();
        }

        renderer.SetFontScale(1.0f);
        char buf[64];
        StringPrintF(buf, "Score: %i", m_score);

        renderer.SetColor(Color(1.0f, 1.0f, 1.0f));
        renderer.BindFontShader();
        renderer.DrawText(0.0f, 0.0f, buf);

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

} // bact
