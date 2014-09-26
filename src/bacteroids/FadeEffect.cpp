
#include "FadeEffect.h"
#include "../renderer/Renderer.h"

#include "../math/Vector4.h"
#include "../math/Matrix4.h"
#include "../math/Types.h"

namespace bact
{

    using namespace rob;

    Fade::Fade(const Color &color)
        : m_color(color)
        , m_fade(0.0f)
        , m_deltaFade(0.0f)
        , m_fadeAcceleration(0.0f)
        , m_maxFade(0.5f)
    { }

    void Fade::SetFadeAcceleration(float acc)
    { m_fadeAcceleration = acc; }

    void Fade::Activate(float delta)
    { m_deltaFade = delta; }

    void Fade::Reset()
    {
        m_deltaFade = 0.0f;
        m_fade = 0.0f;
    }

    void Fade::Update(const float deltaTime)
    {
        m_deltaFade += m_fadeAcceleration * deltaTime;
        m_fade += m_deltaFade * deltaTime;
        m_fade = Clamp(m_fade, 0.0f, m_maxFade);
    }

    void Fade::Render(Renderer *renderer)
    {
        if (m_fade < 0.01f) return;
        Color c = m_color;
        c.a = m_fade;
        renderer->SetColor(c);

        const Viewport vp = renderer->GetView().m_viewport;
        renderer->DrawFilledRectangle(vp.x, vp.y, vp.x + vp.w, vp.y + vp.h);

        float w = vp.w / 2.0f;
        float h = vp.h / 2.0f;
        vec4f p0(vp.x - w, vp.y - h, 0.0f, 1.0f);
        vec4f p1(vp.x + w, vp.y + h, 0.0f, 1.0f);

        const mat4f proj = renderer->GetView().m_projection;
        p0 = proj * p0;
        p1 = proj * p1;
        renderer->DrawFilledRectangle(p0.x, p0.y, p1.x, p1.y);
    }

} // bact
