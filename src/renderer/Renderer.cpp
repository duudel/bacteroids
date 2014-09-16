
#include "Renderer.h"
#include "../graphics/Graphics.h"
#include "../graphics/Shader.h"
#include "../graphics/ShaderProgram.h"
#include "../graphics/VertexBuffer.h"
#include "../graphics/Texture.h"
#include "Font.h"
#include "../resource/MasterCache.h"

#include "../math/Math.h"

#include "../Log.h"

#include <GL/glew.h>
#include <cstring>

namespace rob
{

    extern const char * const g_colorVertexShader;
    extern const char * const g_colorFragmentShader;
    extern const char * const g_fontVertexShader;
    extern const char * const g_fontFragmentShader;

    struct ColorVertex
    {
        float x, y;
        float r, g, b, a;
    };

    struct FontVertex
    {
        float x, y, u, v;
        float r, g, b, a;
    };


    ShaderProgramHandle Renderer::CompileShaderProgram(const char * const vert, const char * const frag)
    {
        VertexShaderHandle vs = m_graphics->CreateVertexShader();
        VertexShader *vertShader = m_graphics->GetVertexShader(vs);
        vertShader->SetSource(vert);
        if (!vertShader->Compile())
        {
            char buffer[512];
            vertShader->GetCompileInfo(buffer, 512);
            log::Error(&buffer[0]);
            m_graphics->DestroyVertexShader(vs);
            return InvalidHandle;
        }

        FragmentShaderHandle fs = m_graphics->CreateFragmentShader();
        FragmentShader *fragShader = m_graphics->GetFragmentShader(fs);
        fragShader->SetSource(frag);
        if (!fragShader->Compile())
        {
            char buffer[512];
            fragShader->GetCompileInfo(buffer, 512);
            log::Error(&buffer[0]);
            m_graphics->DestroyVertexShader(vs);
            m_graphics->DestroyFragmentShader(fs);
            return InvalidHandle;
        }

        ShaderProgramHandle p = m_graphics->CreateShaderProgram();
        ShaderProgram *program = m_graphics->GetShaderProgram(p);
        program->SetShaders(vertShader, fragShader);
        if (!program->Link())
        {
            char buffer[512];
            program->GetLinkInfo(buffer, 512);
            log::Error(&buffer[0]);
            m_graphics->DestroyVertexShader(vs);
            m_graphics->DestroyFragmentShader(fs);
            m_graphics->DestroyFragmentShader(p);
            return InvalidHandle;
        }

        m_graphics->DestroyVertexShader(vs);
        m_graphics->DestroyFragmentShader(fs);

        m_graphics->AddProgramUniform(p, m_globals.projection);
        m_graphics->AddProgramUniform(p, m_globals.position);
        m_graphics->AddProgramUniform(p, m_globals.time);
        m_graphics->AddProgramUniform(p, m_globals.texture0);
        return p;
    }

    static const size_t RENDERER_MEMORY = 4 * 1024;
    static const size_t MAX_VERTEX_BUFFER_SIZE = 1 * 1024 * 1024;

    Renderer::Renderer(Graphics *graphics, MasterCache *cache, LinearAllocator &alloc)
        : m_alloc(alloc.Allocate(RENDERER_MEMORY), RENDERER_MEMORY)
        , m_vb_alloc(alloc.Allocate(MAX_VERTEX_BUFFER_SIZE), MAX_VERTEX_BUFFER_SIZE)
        , m_graphics(graphics)
        , m_vertexBuffer(InvalidHandle)
        , m_colorProgram(InvalidHandle)
        , m_color()
        , m_font()
    {
        m_globals.projection    = m_graphics->CreateUniform("u_projection", UniformType::Mat4);
        m_globals.position      = m_graphics->CreateUniform("u_position", UniformType::Vec4);
        m_globals.time          = m_graphics->CreateUniform("u_time", UniformType::Float);
        m_globals.texture0      = m_graphics->CreateUniform("u_texture0", UniformType::Int);
        m_graphics->SetUniform(m_globals.projection, mat4f::Identity);
        m_graphics->SetUniform(m_globals.time, 0.0f);
        m_graphics->SetUniform(m_globals.texture0, 0);

        m_colorProgram = CompileShaderProgram(g_colorVertexShader, g_colorFragmentShader);
        m_fontProgram = CompileShaderProgram(g_fontVertexShader, g_fontFragmentShader);

        m_font = cache->GetFont("lucida_24.fnt");

        m_vertexBuffer = m_graphics->CreateVertexBuffer();
        m_graphics->BindVertexBuffer(m_vertexBuffer);
        VertexBuffer *vb = m_graphics->GetVertexBuffer(m_vertexBuffer);
        vb->Resize(MAX_VERTEX_BUFFER_SIZE, false);
    }

    Renderer::~Renderer()
    {
        m_graphics->DestroyVertexBuffer(m_vertexBuffer);
        if (m_colorProgram != InvalidHandle)
            m_graphics->DestroyShaderProgram(m_colorProgram);
        if (m_fontProgram != InvalidHandle)
            m_graphics->DestroyShaderProgram(m_fontProgram);
    }

    Graphics* Renderer::GetGraphics()
    { return m_graphics; }

    const GlobalUniforms& Renderer::GetGlobals() const
    { return m_globals; }

    void Renderer::GetScreenSize(int *screenW, int *screenH) const
    {
        int x, y, w, h;
        m_graphics->GetViewport(&x, &y, &w, &h);
        *screenW = w;
        *screenH = h;
    }

    void Renderer::SetProjection(const mat4f &projection)
    { m_graphics->SetUniform(m_globals.projection, projection); }

    void Renderer::SetTime(float time)
    { m_graphics->SetUniform(m_globals.time, time); }


    void Renderer::BindShader(ShaderProgramHandle shader)
    { m_graphics->BindShaderProgram(shader); }

    void Renderer::BindColorShader()
    { BindShader(m_colorProgram); }

    void Renderer::BindFontShader()
    { BindShader(m_fontProgram); }

    void Renderer::SetColor(const Color &color)
    { m_color = color; }

    void Renderer::DrawRectangle(float x0, float y0, float x1, float y1)
    {
        const ColorVertex vertices[] =
        {
            { x0, y0, m_color.r, m_color.g, m_color.b, m_color.a },
            { x1, y0, m_color.r, m_color.g, m_color.b, m_color.a },
            { x1, y1, m_color.r, m_color.g, m_color.b, m_color.a },
            { x0, y1, m_color.r, m_color.g, m_color.b, m_color.a }
        };
        m_graphics->BindVertexBuffer(m_vertexBuffer);
        VertexBuffer *buffer = m_graphics->GetVertexBuffer(m_vertexBuffer);
        buffer->Write(0, sizeof(vertices), vertices);
        m_graphics->SetAttrib(0, 2, sizeof(ColorVertex), 0);
        m_graphics->SetAttrib(1, 4, sizeof(ColorVertex), sizeof(float) * 2);
        m_graphics->BindShaderProgram(m_colorProgram);
        m_graphics->DrawLineLoopArrays(0, 4);
    }

    void Renderer::DrawFilledRectangle(float x0, float y0, float x1, float y1)
    {
        const ColorVertex vertices[] =
        {
            { x0, y0, m_color.r, m_color.g, m_color.b, m_color.a },
            { x1, y0, m_color.r, m_color.g, m_color.b, m_color.a },
            { x0, y1, m_color.r, m_color.g, m_color.b, m_color.a },
            { x1, y1, m_color.r, m_color.g, m_color.b, m_color.a }
        };
        m_graphics->BindVertexBuffer(m_vertexBuffer);
        VertexBuffer *buffer = m_graphics->GetVertexBuffer(m_vertexBuffer);
        buffer->Write(0, sizeof(vertices), vertices);
        m_graphics->SetAttrib(0, 2, sizeof(ColorVertex), 0);
        m_graphics->SetAttrib(1, 4, sizeof(ColorVertex), sizeof(float) * 2);
        m_graphics->DrawTriangleStripArrays(0, 4);
    }

    static const size_t CIRCLE_SEGMENTS = 48;
//    static const size_t CIRCLE_SEGMENTS = 320;

    void Renderer::DrawCirlce(float x, float y, float radius)
    {
        const size_t segments = CIRCLE_SEGMENTS;
        const size_t quarter = segments / 4;
        const size_t vertexCount = segments;
        ColorVertex vertices[vertexCount];

        float angle = 0.0f;
        const float deltaAngle = 2.0f * PI_f / segments;
        for (size_t i = 0; i < quarter; i++, angle += deltaAngle)
        {
            float sn, cs;
            rob::SinCos(angle, sn, cs);
            sn *= radius;
            cs *= radius;

            const size_t i0 = i;
            const size_t i1 = i0 + quarter;
            const size_t i2 = i1 + quarter;
            const size_t i3 = i2 + quarter;
            vertices[i0] = { x - cs, y - sn, m_color.r, m_color.g, m_color.b, m_color.a };
            vertices[i1] = { x + sn, y - cs, m_color.r, m_color.g, m_color.b, m_color.a };
            vertices[i2] = { x + cs, y + sn, m_color.r, m_color.g, m_color.b, m_color.a };
            vertices[i3] = { x - sn, y + cs, m_color.r, m_color.g, m_color.b, m_color.a };
        };

        m_graphics->SetUniform(m_globals.position, vec4f(x, y, 0.0f, 1.0f));

        m_graphics->BindVertexBuffer(m_vertexBuffer);
        VertexBuffer *buffer = m_graphics->GetVertexBuffer(m_vertexBuffer);
        buffer->Write(0, sizeof(vertices), vertices);
        m_graphics->SetAttrib(0, 2, sizeof(ColorVertex), 0);
        m_graphics->SetAttrib(1, 4, sizeof(ColorVertex), sizeof(float) * 2);
        m_graphics->DrawLineLoopArrays(0, vertexCount);
    }

    void Renderer::DrawFilledCirlce(float x, float y, float radius)
    {
        const size_t segments = CIRCLE_SEGMENTS;
        const size_t quarter = segments / 4;
        const size_t vertexCount = segments + 2;
        ColorVertex vertices[vertexCount];

        float angle = 0.0f;
        const float deltaAngle = 2.0f * PI_f / segments;
        vertices[0] = { x, y, m_color.r, m_color.g, m_color.b, m_color.a };
        for (size_t i = 0; i < quarter; i++, angle += deltaAngle)
        {
            float sn, cs;
            rob::SinCos(angle, sn, cs);
            sn *= radius;
            cs *= radius;

            const size_t i0 = 1 + i;
            const size_t i1 = i0 + quarter;
            const size_t i2 = i1 + quarter;
            const size_t i3 = i2 + quarter;
            vertices[i0] = { x - cs, y - sn, m_color.r, m_color.g, m_color.b, m_color.a };
            vertices[i1] = { x + sn, y - cs, m_color.r, m_color.g, m_color.b, m_color.a };
            vertices[i2] = { x + cs, y + sn, m_color.r, m_color.g, m_color.b, m_color.a };
            vertices[i3] = { x - sn, y + cs, m_color.r, m_color.g, m_color.b, m_color.a };
        };
        vertices[2 + segments - 1] = vertices[1];

        m_graphics->SetUniform(m_globals.position, vec4f(x, y, 0.0f, 1.0f));

        m_graphics->BindVertexBuffer(m_vertexBuffer);
        VertexBuffer *buffer = m_graphics->GetVertexBuffer(m_vertexBuffer);
        buffer->Write(0, sizeof(vertices), vertices);
        m_graphics->SetAttrib(0, 2, sizeof(ColorVertex), 0);
        m_graphics->SetAttrib(1, 4, sizeof(ColorVertex), sizeof(float) * 2);
        m_graphics->DrawTriangleFanArrays(0, vertexCount);
    }

    void Renderer::DrawFilledCirlce(float x, float y, float radius, const Color &center)
    {
        const size_t segments = CIRCLE_SEGMENTS;
        const size_t quarter = segments / 4;
        const size_t vertexCount = segments + 2;
        ColorVertex vertices[vertexCount];

        float angle = 0.0f;
        const float deltaAngle = 2.0f * PI_f / segments;
        vertices[0] = { x, y, center.r, center.g, center.b, center.a };
        for (size_t i = 0; i < quarter; i++, angle += deltaAngle)
        {
            float sn, cs;
            rob::SinCos(angle, sn, cs);
            sn *= radius;
            cs *= radius;

            const size_t i0 = 1 + i;
            const size_t i1 = i0 + quarter;
            const size_t i2 = i1 + quarter;
            const size_t i3 = i2 + quarter;
            vertices[i0] = { x - cs, y - sn, m_color.r, m_color.g, m_color.b, m_color.a };
            vertices[i1] = { x + sn, y - cs, m_color.r, m_color.g, m_color.b, m_color.a };
            vertices[i2] = { x + cs, y + sn, m_color.r, m_color.g, m_color.b, m_color.a };
            vertices[i3] = { x - sn, y + cs, m_color.r, m_color.g, m_color.b, m_color.a };
        };
        vertices[2 + segments - 1] = vertices[1];

        m_graphics->SetUniform(m_globals.position, vec4f(x, y, 0.0f, 1.0f));

        m_graphics->BindVertexBuffer(m_vertexBuffer);
        VertexBuffer *buffer = m_graphics->GetVertexBuffer(m_vertexBuffer);
        buffer->Write(0, sizeof(vertices), vertices);
        m_graphics->SetAttrib(0, 2, sizeof(ColorVertex), 0);
        m_graphics->SetAttrib(1, 4, sizeof(ColorVertex), sizeof(float) * 2);
        m_graphics->DrawTriangleFanArrays(0, vertexCount);
    }

    void Renderer::AddFontVertex(FontVertex *&vertex, const float x, const float y, const float u, const float v)
    {
        FontVertex &vert = *vertex++;
        vert.x = x; vert.y = y; vert.u = u; vert.v = v;
        vert.r = m_color.r; vert.g = m_color.g; vert.b = m_color.b; vert.a = m_color.a;
    }

    void Renderer::AddFontQuad(FontVertex *&vertex, const Glyph &glyph,
                               float &cursorX, float &cursorY,
                               const size_t textureW, const size_t textureH)
    {
        const float gW = float(glyph.m_width);
        const float gH = float(glyph.m_height);

        const float uvW = gW / textureW;
        const float uvH = -gH / textureH;

        const float uvX = float(glyph.m_x) / textureW;
        const float uvY = -float(glyph.m_y) / textureH;

        const float cX = cursorX + glyph.m_offsetX;
        const float cY = cursorY + glyph.m_offsetY;

        AddFontVertex(vertex, cX,       cY,         uvX,        uvY);
        AddFontVertex(vertex, cX + gW,  cY,         uvX + uvW,  uvY);
        AddFontVertex(vertex, cX,       cY + gH,    uvX,        uvY + uvH);
        AddFontVertex(vertex, cX,       cY + gH,    uvX,        uvY + uvH);
        AddFontVertex(vertex, cX + gW,  cY,         uvX + uvW,  uvY);
        AddFontVertex(vertex, cX + gW,  cY + gH,    uvX + uvW,  uvY + uvH);

        cursorX += glyph.m_advance; // - glyph.m_offsetX;
    }

    void Renderer::DrawText(float x, float y, const char *text)
    {
        if (!m_font.IsReady()) return;

        m_graphics->SetUniform(m_globals.position, vec4f(x, y, 0.0f, 1.0f));
        m_graphics->BindVertexBuffer(m_vertexBuffer);
        VertexBuffer *buffer = m_graphics->GetVertexBuffer(m_vertexBuffer);

        const size_t maxVertexCount = std::strlen(text) * 6;
        FontVertex * const verticesStart = m_vb_alloc.AllocateArray<FontVertex>(maxVertexCount);
        float cursorX = x;
        float cursorY = y;
        while (*text)
        {
            FontVertex *vertex = verticesStart;

            char c = *text++;
            const Glyph &glyph = m_font.GetGlyph(c);

            uint16_t texturePage = glyph.m_textureIdx;
            const TextureHandle textureHandle = m_font.GetTexture(texturePage);
            const Texture *texture = m_graphics->GetTexture(textureHandle);

            const size_t textureW = texture->GetWidth();
            const size_t textureH = texture->GetHeight();

            AddFontQuad(vertex, glyph, cursorX, cursorY, textureW, textureH);
            for (; (c = *text); text++)
            {
                const Glyph &glyph = m_font.GetGlyph(c);
                if (glyph.m_textureIdx != texturePage)
                    break;

                AddFontQuad(vertex, glyph, cursorX, cursorY, textureW, textureH);
            }

            const size_t vertexCount = vertex - verticesStart;
            buffer->Write(0, vertexCount * sizeof(FontVertex), verticesStart);

            m_graphics->BindTexture(0, textureHandle);
            m_graphics->SetAttrib(0, 4, sizeof(FontVertex), 0);
            m_graphics->SetAttrib(1, 4, sizeof(FontVertex), sizeof(float) * 4);
            m_graphics->DrawTriangleArrays(0, vertexCount);
        }
        m_vb_alloc.Reset();
    }

} // rob
