
#include "FadeEffect.h"
#include "../renderer/Renderer.h"

#include "../math/Vector4.h"
#include "../math/Matrix4.h"
#include "../math/Types.h"

namespace bact
{

    using namespace rob;

    FadeEffect::FadeEffect(const Color &color)
        : m_color(color)
        , m_fade(0.0f)
        , m_deltaFade(0.0f)
        , m_fadeAcceleration(0.0f)
        , m_maxFade(0.5f)
    { }

    void FadeEffect::SetFadeAcceleration(float acc)
    { m_fadeAcceleration = acc; }

    void FadeEffect::Activate(float delta)
    { m_deltaFade = delta; }

    void FadeEffect::Reset()
    {
        m_deltaFade = 0.0f;
        m_fade = 0.0f;
    }

    void FadeEffect::Update(const float deltaTime)
    {
        m_deltaFade += m_fadeAcceleration * deltaTime;
        m_fade += m_deltaFade * deltaTime;
        m_fade = Clamp(m_fade, 0.0f, m_maxFade);
    }

    void FadeEffect::Render(Renderer *renderer)
    {
        if (m_fade < 0.01f) return;
        Color c = m_color;
        c.a = m_fade;
        renderer->SetColor(c);

        View view = renderer->GetView();
        const mat4f proj = view.m_projection;

        view.m_projection = mat4f::Identity;
        renderer->SetView(view);

        renderer->DrawFilledRectangle(-1.0f, -1.0f, 1.0f, 1.0f);

        view.m_projection = proj;
        renderer->SetView(view);
    }

} // bact
