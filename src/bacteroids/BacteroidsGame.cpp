
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
            AddLines(1);
        }

        void AddLines(int n)
        {
            m_cursor.x = m_start.x;
            m_cursor.y += m_renderer.GetFontHeight() * n;
        }

        void AddText(const char *str, const float width)
        {
            m_renderer.DrawText(m_cursor.x + width, m_cursor.y, str);
            m_cursor.x += width + m_renderer.GetTextWidth(str);
        }

        void DrawCursor(const float x, const float y)
        {
            m_renderer.BindColorShader();
            m_renderer.DrawLine(x, y, x, y + m_renderer.GetFontHeight());
            m_renderer.BindFontShader();
        }

        void AddTextInput(const TextInput &input, const float width)
        {
            const float cx = m_cursor.x + width;
            const float cy = m_cursor.y;
            m_renderer.DrawText(cx, cy, input.GetText());

            const float cursorX = m_renderer.GetTextWidth(input.GetText(), input.GetCursor());
            DrawCursor(cx + cursorX, cy);

            m_cursor.x = cx + m_renderer.GetTextWidth(input.GetText());
        }

        void AddTextAlignL(const char *str, const float width)
        {
            m_renderer.DrawText(m_cursor.x + width, m_cursor.y, str);
        }

        void AddTextInputAlignL(const TextInput &input, const float width)
        {
            const float cx = m_cursor.x + width;
            const float cy = m_cursor.y;
            m_renderer.DrawText(cx, cy, input.GetText());

            const float cursorX = m_renderer.GetTextWidth(input.GetText(), input.GetCursor());
            DrawCursor(cx + cursorX, cy);
        }

        void AddTextAlignC(const char *str, const float width)
        {
            const float tw = m_renderer.GetTextWidth(str);
            m_renderer.DrawText(m_cursor.x + (width - tw / 2.0f), m_cursor.y, str);
            m_cursor.x += width;
        }

        void AddTextInputAlignC(const TextInput &input, const float width)
        {
            const float tw = m_renderer.GetTextWidth(input.GetText());
            const float cx = m_cursor.x + (width - tw / 2.0f);
            const float cy = m_cursor.y;
            m_renderer.DrawText(cx, cy, input.GetText());

            const float cursorX = m_renderer.GetTextWidth(input.GetText(), input.GetCursor());
            DrawCursor(cx + cursorX, cy);

            m_cursor.x += width;
        }

        void AddTextAlignR(const char *str, const float width)
        {
            const float tw = m_renderer.GetTextWidth(str);
            m_renderer.DrawText(m_cursor.x + width - tw, m_cursor.y, str);
            m_cursor.x += width;
        }

        void AddTextInputAlignR(const TextInput &input, const float width)
        {
            const float tw = m_renderer.GetTextWidth(input.GetText());
            const float cx = m_cursor.x + width - tw;
            const float cy = m_cursor.y;
            m_renderer.DrawText(cx, cy, input.GetText());

            const float cursorX = m_renderer.GetTextWidth(input.GetText(), input.GetCursor());
            DrawCursor(cx + cursorX, cy);

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
            return true;
        }

        ~MenuState()
        { }

        void Render() override
        {
            Delay(20);

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
            layout.AddTextAlignL("[space]", -100.0f);
            layout.AddTextAlignL("New game", 0.0f);
            layout.AddLine();
            layout.AddTextAlignL("[return]", -100.0f);
            layout.AddTextAlignL("High scores", 0.0f);
            layout.AddLine();
            layout.AddTextAlignL("[esc]", -100.0f);
            layout.AddTextAlignL("Quit", 0.0f);
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
            m_nameInput.SetLengthLimit(HighScoreList::MAX_NAME_LENGTH - 1);
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
            Delay(20);

            Renderer &renderer = GetRenderer();
            renderer.SetView(GetDefaultView());
            renderer.SetColor(Color(1.0f, 1.0f, 1.0f));
            renderer.BindFontShader();

            const Viewport vp = renderer.GetView().m_viewport;

            TextLayout layout(renderer, vp.w / 2.0f, 10.0f);

            renderer.SetFontScale(3.0f);
            layout.AddTextAlignC("High scores", 0.0f);
            layout.AddLines(2);

            renderer.SetFontScale(1.2f);
            renderer.SetColor(Color(1.0f, 1.0f, 1.0f));
            const HighScoreList &highScores = m_gameData.m_highScores;

            char buf[20];
            for (size_t i = 0; i < highScores.GetScoreCount(); i++)
            {
                StringPrintF(buf, "%i. ", i + 1);
                if (i == m_scoreIndex)
                {
                    renderer.SetColor(Color(0.25f, 1.0f, 0.25f));
                    layout.AddTextAlignL(buf, -260.0f);
                    layout.AddTextInputAlignL(m_nameInput, -200.0f);
                    renderer.SetColor(Color(1.0f, 1.0f, 1.0f));
                }
                else
                {
                    layout.AddTextAlignL(buf, -260.0f);
                    layout.AddTextAlignL(highScores.GetName(i), -200.0f);
                }
                StringPrintF(buf, "%i", highScores.GetScore(i));
                layout.AddTextAlignR(buf, 200.0f);
                layout.AddLine();
            }

            const int emptyLines = HighScoreList::MAX_SCORE_COUNT - highScores.GetScoreCount();
            layout.AddLines(emptyLines);

            renderer.SetFontScale(1.0f);
            layout.AddLine();

            renderer.SetColor(Color(1.0f, 1.0f, 1.0f));
            if (InsertingNewScore())
            {
                layout.AddTextAlignC("Press [return] after entering name", 0.0f);
            }
            else
            {
                layout.AddTextAlignC("Press [return] or [esc] to return to main menu", 0.0f);
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
