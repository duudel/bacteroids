
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
            : m_fontShader(InvalidHandle)
        { }

        bool Initialize() override
        {
            m_fontShader = GetRenderer().CompileShaderProgram(g_fontShader.m_vertexShader,
                                                              g_fontShader.m_fragmentShader);

//            float r = 0.05f, g = 0.13f, b = 0.15f, s = 0.5f;
//            GetRenderer().GetGraphics()->SetClearColor(r*s, g*s, b*s);

            return true;
        }

        ~MenuState()
        {
            GetRenderer().GetGraphics()->DestroyShaderProgram(m_fontShader);
        }

        void Render() override
        {
            Renderer &renderer = GetRenderer();
            renderer.SetView(GetDefaultView());
            renderer.SetFontScale(4.0f);
            renderer.SetColor(Color(1.0f, 1.0f, 1.0f));
            renderer.BindFontShader();

            const Viewport vp = renderer.GetView().m_viewport;

            const float fontH = renderer.GetFontHeight();
            const char * const bacteroids = "Bacteroids";
            const float bacteroidsW = renderer.GetTextWidth(bacteroids);
            renderer.DrawText((vp.w - bacteroidsW) / 2.0f, vp.h / 3.0f, bacteroids);

            renderer.SetFontScale(1.0f);
            const char * const instruction = "Press [space] to start";
            const float textW = renderer.GetTextWidth(instruction);
            renderer.DrawText((vp.w - textW) / 2.0f, vp.h / 3.0f + fontH, instruction);
        }

        void OnKeyPress(Keyboard::Key key, Keyboard::Scancode scancode, uint32_t mods) override
        {
            if (key == Keyboard::Key::Escape)
                QuitState();
            if (key == Keyboard::Key::Space)
                ChangeState(2);
        }
    private:
        ShaderProgramHandle m_fontShader;
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
        if (state == 1) ChangeState<MenuState>();
        if (state == 2) ChangeState<BacteroidsState>();
        if (state == 3) m_state->QuitState();
    }

} // bact
