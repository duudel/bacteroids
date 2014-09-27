
#include "BacteroidsGame.h"
#include "BacteroidsState.h"

#include "../application/Window.h"

#include "../graphics/Graphics.h"

#include "Shaders.h"

namespace bact
{

    using namespace rob;

    class MenuState : public GameState
    {
    public:
        MenuState()
        { }

        bool Initialize() override
        {
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

            renderer.SetFontScale(4.0f);
            const float titleH = renderer.GetFontHeight();
            const char * const bacteroids = "Bacteroids";
            const float bacteroidsW = renderer.GetTextWidth(bacteroids);
            renderer.DrawText((vp.w - bacteroidsW) / 2.0f, vp.h / 3.0f, bacteroids);

            renderer.SetFontScale(1.0f);
            const float fontH = renderer.GetFontHeight();
            const char * const instruction = "Press [space] to start";
            const float textW = renderer.GetTextWidth(instruction);
            renderer.DrawText((vp.w - textW) / 2.0f, vp.h / 3.0f + titleH, instruction);

            const char * const instruction2 = "Press [return] for high scores";
            const float textW2 = renderer.GetTextWidth(instruction2);
            renderer.DrawText((vp.w - textW2) / 2.0f, vp.h / 3.0f + titleH + fontH, instruction2);
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
    };


    class HighScoreState : public GameState
    {
    public:
        HighScoreState()
        { }

        bool Initialize() override
        {
            return true;
        }

        ~HighScoreState()
        {
        }

        void Render() override
        {
            Renderer &renderer = GetRenderer();
            renderer.SetView(GetDefaultView());
            renderer.SetColor(Color(1.0f, 1.0f, 1.0f));
            renderer.BindFontShader();

            const Viewport vp = renderer.GetView().m_viewport;

            renderer.SetFontScale(4.0f);
            const float titleH = renderer.GetFontHeight();
            const char * const highScores = "High scores";
            const float highScoresW = renderer.GetTextWidth(highScores);
            renderer.DrawText((vp.w - highScoresW) / 2.0f, vp.h / 5.0f, highScores);

            renderer.SetFontScale(1.0f);
            const float fontH = renderer.GetFontHeight();
            const char * const instruction2 = "Press [return] after entering name:";
            const float textW2 = renderer.GetTextWidth(instruction2);
            renderer.DrawText((vp.w - textW2) / 2.0f, vp.h / 3.0f + titleH, instruction2);

            const float nameW = renderer.GetTextWidth(m_nameInput.GetText());
            const float x = (vp.w - nameW) / 2.0f;
            const float y = vp.h / 3.0f + titleH + fontH;
            renderer.DrawText(x, y, m_nameInput.GetText());
        }

        void OnTextInput(const char *str) override
        {
            TextInputHandler(m_nameInput).OnTextInput(str);
        }

        void OnKeyPress(Keyboard::Key key, Keyboard::Scancode scancode, uint32_t mods) override
        {
            if (key == Keyboard::Key::Return && m_nameInput.GetLength() > 0)
            {
                ChangeState(1);
            }
        }

        void OnKeyDown(Keyboard::Key key, Keyboard::Scancode scancode, uint32_t mods) override
        {
            TextInputHandler(m_nameInput).OnKeyDown(key, mods);
        }
    private:
        TextInput m_nameInput;
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

        m_highScores.Load();

        ChangeState<MenuState>();
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
        case 0: break;

        case 1: ChangeState<MenuState>(); break;
        case 2: ChangeState<BacteroidsState>(); break;
        case 3: ChangeState<HighScoreState>(); break;
        case 4: m_state->QuitState(); break;
        default:
            log::Error("Invalid state change (", state, ")");
            break;
        }
    }

} // bact
