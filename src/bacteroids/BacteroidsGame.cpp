
#include "BacteroidsGame.h"
#include "BacteroidsState.h"

#include "../application/Window.h"

#include "../graphics/Graphics.h"

#include "Shaders.h"

namespace bact
{

    using namespace rob;

    struct TextLayout
    {
        Renderer &m_renderer;
        vec2f m_cursor;
        vec2f m_start;

        TextLayout(Renderer &renderer, float x, float y)
            : m_renderer(renderer)
            , m_cursor(x, y)
            , m_start(x, y)
        { }

        void AddLine()
        {
            m_cursor.x = m_start.x;
            m_cursor.y += m_renderer.GetFontHeight();
        }

        void AddText(const char *str, const float width)
        {
            m_renderer.DrawText(m_cursor.x + width, m_cursor.y, str);
            m_cursor.x += width + m_renderer.GetTextWidth(str);
        }

        void AddTextAlignC(const char *str, const float width)
        {
            const float tw = m_renderer.GetTextWidth(str);
            m_renderer.DrawText(m_cursor.x + (width - tw / 2.0f), m_cursor.y, str);
            m_cursor.x += width;
        }

        void AddTextAlignR(const char *str, const float width)
        {
            const float tw = m_renderer.GetTextWidth(str);
            m_renderer.DrawText(m_cursor.x + width - tw, m_cursor.y, str);
            m_cursor.x += width;
        }
    };

    class MenuState : public GameState
    {
    public:
        MenuState(GameData &gameData)
            : m_gameData(gameData)
        { }

        bool Initialize() override
        {
            m_gameData.m_score = -1;
//            float r = 0.05f, g = 0.13f, b = 0.15f, s = 0.5f;
//            GetRenderer().GetGraphics()->SetClearColor(r*s, g*s, b*s);

            return true;
        }

        ~MenuState()
        {
        }

        void Render() override
        {
            Renderer &renderer = GetRenderer();
            renderer.SetView(GetDefaultView());
            renderer.SetColor(Color(1.0f, 1.0f, 1.0f));
            renderer.BindFontShader();

            const Viewport vp = renderer.GetView().m_viewport;
            TextLayout layout(renderer, vp.w / 2.0f, vp.h / 3.0f);

            renderer.SetFontScale(4.0f);
            layout.AddTextAlignC("Bacteroids", 0.0f);
            layout.AddLine();

            renderer.SetFontScale(1.0f);
            layout.AddTextAlignC("Press [space] to start", 0.0f);
            layout.AddLine();
            layout.AddTextAlignC("Press [return] to show high scores", 0.0f);
            layout.AddLine();
            layout.AddTextAlignC("Press [esc] to exit", 0.0f);
            layout.AddLine();
        }

        void OnKeyPress(Keyboard::Key key, Keyboard::Scancode scancode, uint32_t mods) override
        {
            if (key == Keyboard::Key::Escape)
                QuitState();
            if (key == Keyboard::Key::Space)
                ChangeState(2);
            if (key == Keyboard::Key::Return)
                ChangeState(3);
        }
    private:
        GameData &m_gameData;
    };


    class HighScoreState : public GameState
    {
    public:
        HighScoreState(GameData &gameData)
            : m_gameData(gameData)
            , m_scoreIndex(HighScoreList::INVALID_INDEX)
            , m_nameInput()
        {
            m_nameInput.SetLengthLimit(HighScoreList::MAX_NAME_LENGTH);
        }

        bool Initialize() override
        {
            const int score = m_gameData.m_score;
            if (score != -1)
            {
                m_scoreIndex = m_gameData.m_highScores.AddScore(score);
            }
            return true;
        }

        ~HighScoreState()
        { }

        bool InsertingNewScore() const
        { return m_scoreIndex != HighScoreList::INVALID_INDEX; }

        void Render() override
        {
            Renderer &renderer = GetRenderer();
            renderer.SetView(GetDefaultView());
            renderer.SetColor(Color(1.0f, 1.0f, 1.0f));
            renderer.BindFontShader();

            const Viewport vp = renderer.GetView().m_viewport;

            TextLayout layout(renderer, vp.w / 2.0f, 10.0f);

            renderer.SetFontScale(3.0f);
            layout.AddTextAlignC("High scores", 0.0f);
            layout.AddLine();

            renderer.SetFontScale(1.0f);
            layout.AddLine();

            if (InsertingNewScore())
            {
                layout.AddTextAlignC("Press [return] after entering name", 0.0f);
            }
            else
            {
                layout.AddTextAlignC("Press [return] or [esc] to return to main menu", 0.0f);
            }
            layout.AddLine();
            layout.AddLine();

            renderer.SetFontScale(1.2f);
            const HighScoreList &highScores = m_gameData.m_highScores;

            char buf[20];
            for (size_t i = 0; i < highScores.GetScoreCount(); i++)
            {
                if (i == m_scoreIndex)
                {
                    renderer.SetColor(Color(0.05f, 1.0f, 0.05f));
                    layout.AddTextAlignR(m_nameInput.GetText(), -10.0f);
                }
                else
                {
                    renderer.SetColor(Color(1.0f, 1.0f, 1.0f));
                    layout.AddTextAlignR(highScores.GetName(i), -10.0f);
                }
                StringPrintF(buf, "%i", highScores.GetScore(i));
                layout.AddText(buf, 10.0f);
                layout.AddLine();
            }
        }

        void OnTextInput(const char *str) override
        {
            TextInputHandler(m_nameInput).OnTextInput(str);
        }

        void OnKeyPress(Keyboard::Key key, Keyboard::Scancode scancode, uint32_t mods) override
        {
            if (key == Keyboard::Key::Return)
            {
                if (InsertingNewScore() && m_nameInput.GetLength() > 0)
                {
                    m_gameData.m_highScores.SetName(m_scoreIndex, m_nameInput.GetText());
                    m_gameData.m_highScores.Save();
                    m_scoreIndex = HighScoreList::INVALID_INDEX;
                }
                else
                {
                    ChangeState(1);
                }
            }
            else if (key == Keyboard::Key::Escape)
            {
                ChangeState(1);
            }
        }

        void OnKeyDown(Keyboard::Key key, Keyboard::Scancode scancode, uint32_t mods) override
        {
            TextInputHandler(m_nameInput).OnKeyDown(key, mods);
        }
    private:
        GameData &m_gameData;
        size_t m_scoreIndex;
        TextInput m_nameInput;
    };


    Bacteroids::Bacteroids()
        : m_gameData()
    { }

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

        m_gameData.m_highScores.Load();

        HandleStateChange(STATE_MENU);
        return true;
    }

    void Bacteroids::OnKeyPress(Keyboard::Key key, Keyboard::Scancode scancode, uint32_t mods)
    {
        if (key == Keyboard::Key::LAlt)
            m_window->ToggleGrabMouse();
        Game::OnKeyPress(key, scancode, mods);
    }

    void Bacteroids::HandleStateChange(int state)
    {
        switch (state)
        {
        case STATE_NO_CHANGE: break;

        case STATE_MENU:        ChangeState<MenuState>(m_gameData); break;
        case STATE_GAME:        ChangeState<BacteroidsState>(m_gameData); break;
        case STATE_HIGH_SCORE:  ChangeState<HighScoreState>(m_gameData); break;
        default:
            log::Error("Invalid state change (", state, ")");
            break;
        }
    }

} // bact
