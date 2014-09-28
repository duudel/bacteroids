
#include "BacteroidsState.h"

#include "Shaders.h"
#include "TextLayout.h"

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
    static const Rect PLAY_AREA(PLAY_AREA_LEFT, PLAY_AREA_BOTTOM,
                                PLAY_AREA_RIGHT, PLAY_AREA_TOP);


    BacteroidsState::BacteroidsState(GameData &gameData)
        : m_gameData(gameData)
        , m_random()
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
        GetWindow().GrabMouse();

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

        Player::shader = m_playerShader;
        Bacter::shader = m_bacterShader;
        Projectile::shader = m_projectileShader;

        m_soundPlayer.Init(GetAudio(), GetCache());

        m_player.SetPosition(0.0f, 0.0f);
        m_objects.Init(GetAllocator());
        m_objects.AddObject(&m_player);

        m_quadTree = GetAllocator().AllocateArray<GameObject*>(MAX_OBJECTS);

        m_score = 0;
        m_kills = 0;

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
            TogglePause();
        if (m_time.IsPaused())
        {
            if (key == Keyboard::Key::Q)
                ChangeState(STATE_MENU);
        }

        if (key == Keyboard::Key::M)
            GetAudio().ToggleMute();

        if (m_player.IsDead())
        {
            if (key == Keyboard::Key::Space)
            {
                m_gameData.m_score = m_score;
                ChangeState(STATE_HIGH_SCORE);
            }
        }
    }

    void BacteroidsState::OnKeyDown(Keyboard::Key key, Keyboard::Scancode scancode, uint32_t mods)
    { m_input.SetKey(scancode, true); }

    void BacteroidsState::OnKeyUp(Keyboard::Key key, Keyboard::Scancode scancode, uint32_t mods)
    { m_input.SetKey(scancode, false); }


    void BacteroidsState::SpawnBacter(float distMod /*= 1.0f*/)
    {
        const float D = vec2f(PLAY_AREA_RIGHT, PLAY_AREA_TOP).Length() * distMod + 1.5f;

        if (!m_objects.CanObtainBacter()) return;

        Bacter *bacter = m_objects.ObtainBacter();
        bacter->SetTarget(&m_player);
        bacter->RandomizeAnimation(m_random);
        bacter->SetPosition(m_random.GetDirection() * D);
    }

    void BacteroidsState::SplitBacter(Bacter *bacter)
    {
        if (!bacter->DiesIfSplits())
        {
            if (m_objects.CanObtainBacter())
            {
                Bacter *other = m_objects.ObtainBacter();
                other->CopyAttributes(bacter);
                other->Split(m_random);
            }
        }
        bacter->Split(m_random);
        vec2f p = bacter->GetPosition();
        m_soundPlayer.PlayBacterSplitSound(p.x, p.y);
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
            if (me->CanSplit())
            {
                m_score += me->GetPoints();
                m_kills++;
                me->AddVelocity(obj->GetVelocity() * 0.5f);
                SplitBacter(me);
            }
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
            me->TakeHit(m_soundPlayer);
            m_damageFade.Activate(1.0f);
            if (me->IsDead())
            {
                m_damageFade.SetFadeAcceleration(0.0f);
                m_damageFade.Activate(1.0f);
                m_soundPlayer.PlayPlayerDeathSound(p.x, p.y);
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
    #include "CollisionTesting.inl"
    }

    void BacteroidsState::UpdatePlayer(const GameTime &gameTime)
    {
        if (!m_player.IsAlive()) return;

        m_input.UpdateMouse();
        m_player.Update(gameTime, m_input, m_objects, m_soundPlayer);

        vec2f pl_pos = m_player.GetPosition();
        const float pl_radius = m_player.GetRadius();

        pl_pos.x = Clamp(pl_pos.x, PLAY_AREA_LEFT + pl_radius, PLAY_AREA_RIGHT - pl_radius);
        pl_pos.y = Clamp(pl_pos.y, PLAY_AREA_BOTTOM + pl_radius, PLAY_AREA_TOP - pl_radius);

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

        m_soundPlayer.UpdateTime(gameTime);

        UpdatePlayer(gameTime);

        DoCollisions();

        for (size_t i = 0; i < m_objects.Size(); i++)
        {
            m_objects[i]->Update(gameTime, PLAY_AREA);
            // TODO: Spontaneous splitting is very much a hack I says. Fix pls.
            if (m_objects[i]->GetType() == Bacter::TYPE)
            {
                Bacter *bacter = (Bacter*)m_objects[i];
                if (bacter->WantsToSplit() && m_objects.CanObtainBacter())
                    SplitBacter(bacter);
            }
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

        m_damageFade.Update(gameTime.GetDeltaSeconds());
    }

    void BacteroidsState::RenderPause()
    {
        Renderer &renderer = GetRenderer();
        renderer.SetColor(Color(1.0f, 1.0f, 1.0f));
        renderer.BindFontShader();

        const Viewport vp = renderer.GetView().m_viewport;

        TextLayout layout(renderer, vp.w / 2.0f, vp.h / 3.0f);

        renderer.SetFontScale(4.0f);
        layout.AddTextAlignC("Game paused", 0.0f);
        layout.AddLine();

        renderer.SetFontScale(1.0f);
        layout.AddTextAlignC("[Esc] to resume", 0.0f);
        layout.AddLine();
        layout.AddTextAlignC("[Q] to exit to main menu", 0.0f);
        layout.AddLines(2);
        layout.AddTextAlignC("[M] to mute/unmute", 0.0f);
    }

    void BacteroidsState::RenderGameOver()
    {
        Renderer &renderer = GetRenderer();
        renderer.SetColor(Color(1.0f, 1.0f, 1.0f));
        renderer.BindFontShader();

        const Viewport vp = renderer.GetView().m_viewport;

        TextLayout layout(renderer, vp.w / 2.0f, vp.h / 3.0f);

        renderer.SetFontScale(4.0f);
        layout.AddTextAlignC("Game over", 0.0f);
        layout.AddLine();

        renderer.SetFontScale(1.0f);
        layout.AddTextAlignC("Press [space] to continue", 0.0f);
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
            float r = obj->GetRadius() * 1.1f;

            if ((p.x > PLAY_AREA_LEFT - r && p.x < PLAY_AREA_RIGHT + r &&
                 p.y > PLAY_AREA_BOTTOM - r && p.y < PLAY_AREA_TOP + r) == false)
            {
                continue;
            }

            renderer.BindShader(obj->GetShader());

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

        TextLayout layout(renderer, 0.0f, 0.0f);

        renderer.BindFontShader();
        renderer.SetColor(Color(1.0f, 1.0f, 1.0f));
        renderer.SetFontScale(1.0f);

        char buf[64];
        StringPrintF(buf, "Score: %i", m_score);
        layout.AddText(buf, 0.0f);
        layout.AddLine();

        StringPrintF(buf, "Kills: %i", m_kills);
        layout.AddText(buf, 0.0f);
        layout.AddLine();

        const int ptPerKill = (m_kills) ? (m_score / m_kills) : 0;
        StringPrintF(buf, "pt per kill: %i", ptPerKill);
        layout.AddText(buf, 0.0f);
        layout.AddLine();

        const float hx = 10.0f;
        const float hy = layout.m_cursor.y + 10.0f;

        renderer.BindColorShader();
        renderer.SetColor(Color(0.5f, 0.5f, 0.5f));
        renderer.DrawFilledRectangle(hx, hy, hx + 100.0f, hy + 10.0f);
        renderer.SetColor(Color(0.85f, 0.05f, 0.05f));
        renderer.DrawFilledRectangle(hx, hy, hx + m_player.GetHealth(), hy + 10.0f);
    }

} // bact
