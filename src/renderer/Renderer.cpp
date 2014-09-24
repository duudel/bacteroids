
#include "Renderer.h"
#include "../graphics/Graphics.h"
#include "../graphics/Shader.h"
#include "../graphics/ShaderProgram.h"
#include "../graphics/VertexBuffer.h"
#include "../graphics/Texture.h"

#include "../resource/MasterCache.h"

#include "../math/Math.h"

#include "../Log.h"
#include "../String.h"

#include <GL/glew.h>

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
        , m_globals()
        , m_vertexBuffer(InvalidHandle)
        , m_colorProgram(InvalidHandle)
        , m_fontProgram(InvalidHandle)
        , m_color(Color::White)
        , m_font()
    {
        m_globals.projection    = m_graphics->CreateGlobalUniform("u_projection", UniformType::Mat4);
        m_globals.position      = m_graphics->CreateGlobalUniform("u_position", UniformType::Vec4);
        m_globals.time          = m_graphics->CreateGlobalUniform("u_time", UniformType::Float);
        m_globals.texture0      = m_graphics->CreateGlobalUniform("u_texture0", UniformType::Int);
        m_graphics->SetUniform(m_globals.projection, mat4f::Identity);
        m_graphics->SetUniform(m_globals.time, 0.0f);
        m_graphics->SetUniform(m_globals.texture0, 0);

        m_colorProgram = CompileShaderProgram(g_colorVertexShader, g_colorFragmentShader);
        m_fontProgram = CompileShaderProgram(g_fontVertexShader, g_fontFragmentShader);

//        m_font = cache->GetFont("lucida_24.fnt");
        m_font = cache->GetFont("dejavu_24.fnt");

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
        m_graphics->DecRefUniform(m_globals.projection);
        m_graphics->DecRefUniform(m_globals.position);
        m_graphics->DecRefUniform(m_globals.time);
        m_graphics->DecRefUniform(m_globals.texture0);
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

    void Renderer::DrawLine(float x0, float y0, float x1, float y1)
    {
        const float dx = x1 - x0;
        const float dy = y1 - y0;
        const size_t vertexCount = 2;
        ColorVertex* vertices = m_vb_alloc.AllocateArray<ColorVertex>(vertexCount);
        vertices[0] = { 0.0f, 0.0f, m_color.r, m_color.g, m_color.b, m_color.a };
        vertices[1] = { dx, dy, m_color.r, m_color.g, m_color.b, m_color.a };

        m_graphics->SetUniform(m_globals.position, vec4f(x0, y0, 0.0f, 1.0f));

        m_graphics->BindVertexBuffer(m_vertexBuffer);
        VertexBuffer *buffer = m_graphics->GetVertexBuffer(m_vertexBuffer);
        buffer->Write(0, vertexCount * sizeof(ColorVertex), vertices);
        m_graphics->SetAttrib(0, 2, sizeof(ColorVertex), 0);
        m_graphics->SetAttrib(1, 4, sizeof(ColorVertex), sizeof(float) * 2);
        m_graphics->DrawLineLoopArrays(0, vertexCount);

        m_vb_alloc.Reset();
    }

    void Renderer::DrawRectangle(float x0, float y0, float x1, float y1)
    {
        const float w = x1 - x0;
        const float h = y1 - y0;
        const size_t vertexCount = 4;
        ColorVertex* vertices = m_vb_alloc.AllocateArray<ColorVertex>(vertexCount);
        vertices[0] = { 0.0f, 0.0f, m_color.r, m_color.g, m_color.b, m_color.a };
        vertices[1] = { w, 0.0f, m_color.r, m_color.g, m_color.b, m_color.a };
        vertices[2] = { 0.0f, h, m_color.r, m_color.g, m_color.b, m_color.a };
        vertices[3] = { w, h, m_color.r, m_color.g, m_color.b, m_color.a };

        m_graphics->SetUniform(m_globals.position, vec4f(x0, y0, 0.0f, 1.0f));

        m_graphics->BindVertexBuffer(m_vertexBuffer);
        VertexBuffer *buffer = m_graphics->GetVertexBuffer(m_vertexBuffer);
        buffer->Write(0, vertexCount * sizeof(ColorVertex), vertices);
        m_graphics->SetAttrib(0, 2, sizeof(ColorVertex), 0);
        m_graphics->SetAttrib(1, 4, sizeof(ColorVertex), sizeof(float) * 2);
        m_graphics->DrawLineLoopArrays(0, vertexCount);

        m_vb_alloc.Reset();
    }

    void Renderer::DrawFilledRectangle(float x0, float y0, float x1, float y1)
    {
        const float w = x1 - x0;
        const float h = y1 - y0;
        const size_t vertexCount = 4;
        ColorVertex* vertices = m_vb_alloc.AllocateArray<ColorVertex>(vertexCount);
        vertices[0] = { 0.0f, 0.0f, m_color.r, m_color.g, m_color.b, m_color.a };
        vertices[1] = { w, 0.0f, m_color.r, m_color.g, m_color.b, m_color.a };
        vertices[2] = { 0.0f, h, m_color.r, m_color.g, m_color.b, m_color.a };
        vertices[3] = { w, h, m_color.r, m_color.g, m_color.b, m_color.a };

        m_graphics->SetUniform(m_globals.position, vec4f(x0, y0, 0.0f, 1.0f));

        m_graphics->BindVertexBuffer(m_vertexBuffer);
        VertexBuffer *buffer = m_graphics->GetVertexBuffer(m_vertexBuffer);
        buffer->Write(0, vertexCount * sizeof(ColorVertex), vertices);
        m_graphics->SetAttrib(0, 2, sizeof(ColorVertex), 0);
        m_graphics->SetAttrib(1, 4, sizeof(ColorVertex), sizeof(float) * 2);
        m_graphics->DrawTriangleStripArrays(0, vertexCount);

        m_vb_alloc.Reset();
    }

    static const size_t CIRCLE_SEGMENTS = 48;
    static const float SEG_RADIUS_SCALE = 1.0f;

//    static ColorVertex* g_circleCache[16] = { 0 };
//    static ColorVertex g_circleCacheData[16 * 64] = { 0 };

    void Renderer::DrawCirlce(float x, float y, float radius)
    {
        const size_t segs = CIRCLE_SEGMENTS * (radius / SEG_RADIUS_SCALE);
        const size_t segments = Min((segs + 3) & ~0x3, CIRCLE_SEGMENTS);
        const size_t quarter = segments / 4;
        const size_t vertexCount = segments;
        ColorVertex* vertices = m_vb_alloc.AllocateArray<ColorVertex>(vertexCount);

        float angle = 0.0f;
        const float deltaAngle = 2.0f * PI_f / segments;
        for (size_t i = 0; i < quarter; i++, angle += deltaAngle)
        {
            float sn, cs;
            rob::FastSinCos(angle, sn, cs);
            sn *= radius;
            cs *= radius;

            const size_t i0 = i;
            const size_t i1 = i0 + quarter;
            const size_t i2 = i1 + quarter;
            const size_t i3 = i2 + quarter;
            vertices[i0] = { -cs, -sn, m_color.r, m_color.g, m_color.b, m_color.a };
            vertices[i1] = { +sn, -cs, m_color.r, m_color.g, m_color.b, m_color.a };
            vertices[i2] = { +cs, +sn, m_color.r, m_color.g, m_color.b, m_color.a };
            vertices[i3] = { -sn, +cs, m_color.r, m_color.g, m_color.b, m_color.a };
        };

        m_graphics->SetUniform(m_globals.position, vec4f(x, y, 0.0f, 1.0f));

        m_graphics->BindVertexBuffer(m_vertexBuffer);
        VertexBuffer *buffer = m_graphics->GetVertexBuffer(m_vertexBuffer);
        buffer->Write(0, vertexCount * sizeof(ColorVertex), vertices);
        m_graphics->SetAttrib(0, 2, sizeof(ColorVertex), 0);
        m_graphics->SetAttrib(1, 4, sizeof(ColorVertex), sizeof(float) * 2);
        m_graphics->DrawLineLoopArrays(0, vertexCount);

        m_vb_alloc.Reset();
    }

    void Renderer::DrawFilledCirlce(float x, float y, float radius)
    {
        const size_t segs = CIRCLE_SEGMENTS * (radius / SEG_RADIUS_SCALE);
        const size_t segments = Min((segs + 3) & ~0x3, CIRCLE_SEGMENTS);
        const size_t quarter = segments / 4;
        const size_t vertexCount = segments + 2;
        ColorVertex* vertices = m_vb_alloc.AllocateArray<ColorVertex>(vertexCount);

        float angle = 0.0f;
        const float deltaAngle = 2.0f * PI_f / segments;
        vertices[0] = { 0.0f, 0.0f, m_color.r, m_color.g, m_color.b, m_color.a };
        for (size_t i = 0; i < quarter; i++, angle += deltaAngle)
        {
            float sn, cs;
            rob::FastSinCos(angle, sn, cs);
            sn *= radius;
            cs *= radius;

            const size_t i0 = 1 + i;
            const size_t i1 = i0 + quarter;
            const size_t i2 = i1 + quarter;
            const size_t i3 = i2 + quarter;
            vertices[i0] = { -cs, -sn, m_color.r, m_color.g, m_color.b, m_color.a };
            vertices[i1] = { +sn, -cs, m_color.r, m_color.g, m_color.b, m_color.a };
            vertices[i2] = { +cs, +sn, m_color.r, m_color.g, m_color.b, m_color.a };
            vertices[i3] = { -sn, +cs, m_color.r, m_color.g, m_color.b, m_color.a };
        };
        vertices[2 + segments - 1] = vertices[1];

        m_graphics->SetUniform(m_globals.position, vec4f(x, y, 0.0f, 1.0f));

        m_graphics->BindVertexBuffer(m_vertexBuffer);
        VertexBuffer *buffer = m_graphics->GetVertexBuffer(m_vertexBuffer);
        buffer->Write(0, vertexCount * sizeof(ColorVertex), vertices);
        m_graphics->SetAttrib(0, 2, sizeof(ColorVertex), 0);
        m_graphics->SetAttrib(1, 4, sizeof(ColorVertex), sizeof(float) * 2);
        m_graphics->DrawTriangleFanArrays(0, vertexCount);

        m_vb_alloc.Reset();
    }

    void Renderer::DrawFilledCirlce(float x, float y, float radius, const Color &center)
    {
        const size_t segs = CIRCLE_SEGMENTS * (radius / SEG_RADIUS_SCALE);
        const size_t segments = Min((segs + 3) & ~0x3, CIRCLE_SEGMENTS);
        const size_t quarter = segments / 4;
        const size_t vertexCount = segments + 2;
        ColorVertex* vertices = m_vb_alloc.AllocateArray<ColorVertex>(vertexCount);

//        ColorVertex* vertices = nullptr;
//        const size_t cacheIdx = segments / 3;
//        if (g_circleCache[cacheIdx])
//            vertices = &g_circleCacheData[cacheIdx * 64];

//        if (!vertices)
//        {
//            vertices = &g_circleCacheData[cacheIdx * 64];//m_vb_alloc.AllocateArray<ColorVertex>(vertexCount);

            float angle = 0.0f;
            const float deltaAngle = 2.0f * PI_f / segments;
            vertices[0] = { 0.0f, 0.0f, center.r, center.g, center.b, center.a };
            for (size_t i = 0; i < quarter; i++, angle += deltaAngle)
            {
                float sn, cs;
                rob::FastSinCos(angle, sn, cs);
                sn *= radius;
                cs *= radius;

                const size_t i0 = 1 + i;
                const size_t i1 = i0 + quarter;
                const size_t i2 = i1 + quarter;
                const size_t i3 = i2 + quarter;
                vertices[i0] = { -cs, -sn, m_color.r, m_color.g, m_color.b, m_color.a };
                vertices[i1] = { +sn, -cs, m_color.r, m_color.g, m_color.b, m_color.a };
                vertices[i2] = { +cs, +sn, m_color.r, m_color.g, m_color.b, m_color.a };
                vertices[i3] = { -sn, +cs, m_color.r, m_color.g, m_color.b, m_color.a };
            };
            vertices[2 + segments - 1] = vertices[1];

//            g_circleCache[cacheIdx] = vertices;
//        }

        m_graphics->SetUniform(m_globals.position, vec4f(x, y, 0.0f, 1.0f));

        m_graphics->BindVertexBuffer(m_vertexBuffer);
        VertexBuffer *buffer = m_graphics->GetVertexBuffer(m_vertexBuffer);
        buffer->Write(0, vertexCount * sizeof(ColorVertex), vertices);
        m_graphics->SetAttrib(0, 2, sizeof(ColorVertex), 0);
        m_graphics->SetAttrib(1, 4, sizeof(ColorVertex), sizeof(float) * 2);
        m_graphics->DrawTriangleFanArrays(0, vertexCount);

//        m_vb_alloc.Reset();
    }


    void Renderer::AddFontVertex(FontVertex *&vertex, const float x, const float y, const float u, const float v)
    {
        FontVertex &vert = *vertex++;
        vert.x = x; vert.y = y; vert.u = u; vert.v = v;
        vert.r = m_color.r; vert.g = m_color.g; vert.b = m_color.b; vert.a = m_color.a;
    }

    void Renderer::AddFontQuad(FontVertex *&vertex, const uint32_t c, const Glyph &glyph,
                               float &cursorX, float &cursorY,
                               const size_t textureW, const size_t textureH)
    {
        if (std::isprint(c) && c != ' ')
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
        }

        cursorX += glyph.m_advance;
    }

    void Renderer::DrawText(float x, float y, const char *text)
    {
        if (!m_font.IsReady()) return;

        m_graphics->SetUniform(m_globals.position, vec4f(x, y, 0.0f, 1.0f));
        m_graphics->BindVertexBuffer(m_vertexBuffer);
        m_graphics->SetAttrib(0, 4, sizeof(FontVertex), 0);
        m_graphics->SetAttrib(1, 4, sizeof(FontVertex), sizeof(float) * 4);
        VertexBuffer *buffer = m_graphics->GetVertexBuffer(m_vertexBuffer);

        const size_t textLen = StringLength(text);
        const size_t maxVertexCount = textLen * 6;
        FontVertex * const verticesStart = m_vb_alloc.AllocateArray<FontVertex>(maxVertexCount);
        float cursorX = x;
        float cursorY = y;

        const char * const end = text + textLen;
        while (*text)
        {
            FontVertex *vertex = verticesStart;

            const uint32_t c = DecodeUtf8(text, end);
            const Glyph &glyph = m_font.GetGlyph(c);

            uint16_t texturePage = glyph.m_textureIdx;
            const TextureHandle textureHandle = m_font.GetTexture(texturePage);
            const Texture *texture = m_graphics->GetTexture(textureHandle);

            const size_t textureW = texture->GetWidth();
            const size_t textureH = texture->GetHeight();

            AddFontQuad(vertex, c, glyph, cursorX, cursorY, textureW, textureH);
            while (text != end)
            {
                const uint32_t c = DecodeUtf8(text, end);
                const Glyph &glyph = m_font.GetGlyph(c);
                if (glyph.m_textureIdx != texturePage)
                    break;
                AddFontQuad(vertex, c, glyph, cursorX, cursorY, textureW, textureH);
            }

            const size_t vertexCount = vertex - verticesStart;
            buffer->Write(0, vertexCount * sizeof(FontVertex), verticesStart);

            m_graphics->BindTexture(0, textureHandle);
            m_graphics->DrawTriangleArrays(0, vertexCount);
        }
        m_vb_alloc.Reset();
    }

    float Renderer::GetTextWidth(const char *text) const
    {
        float width = 0.0f;
        while (*text)
        {
            const uint32_t c = DecodeUtf8(text, 0);
            const Glyph &glyph = m_font.GetGlyph(c);
            width += float(glyph.m_advance);
        }
        return width;
    }

    float Renderer::GetTextWidth(const char *text, size_t charCount) const
    {
        float width = 0.0f;
        for (; *text && charCount; )
        {
            const char * const s = text;
            const uint32_t c = DecodeUtf8(text, 0);
            charCount -= (text - s);
            const Glyph &glyph = m_font.GetGlyph(c);
            width += float(glyph.m_advance);
        }
        return width;
    }

    void Renderer::DrawTextAscii(float x, float y, const char *text)
    {
        if (!m_font.IsReady()) return;

        m_graphics->SetUniform(m_globals.position, vec4f(x, y, 0.0f, 1.0f));
        m_graphics->BindVertexBuffer(m_vertexBuffer);
        m_graphics->SetAttrib(0, 4, sizeof(FontVertex), 0);
        m_graphics->SetAttrib(1, 4, sizeof(FontVertex), sizeof(float) * 4);
        VertexBuffer *buffer = m_graphics->GetVertexBuffer(m_vertexBuffer);

        const size_t textLen = StringLength(text);
        const size_t maxVertexCount = textLen * 6;
        FontVertex * const verticesStart = m_vb_alloc.AllocateArray<FontVertex>(maxVertexCount);
        float cursorX = x;
        float cursorY = y;

        const char * const end = text + textLen;
        while (*text)
        {
            FontVertex *vertex = verticesStart;

            const uint32_t c = uint8_t(*text++);
            const Glyph &glyph = m_font.GetGlyph(c);

            uint16_t texturePage = glyph.m_textureIdx;
            const TextureHandle textureHandle = m_font.GetTexture(texturePage);
            const Texture *texture = m_graphics->GetTexture(textureHandle);

            const size_t textureW = texture->GetWidth();
            const size_t textureH = texture->GetHeight();

            AddFontQuad(vertex, c, glyph, cursorX, cursorY, textureW, textureH);
            while (text != end)
            {
                const uint32_t c = uint8_t(*text++);
                const Glyph &glyph = m_font.GetGlyph(c);
                if (glyph.m_textureIdx != texturePage)
                    break;
                AddFontQuad(vertex, c, glyph, cursorX, cursorY, textureW, textureH);
            }

            const size_t vertexCount = vertex - verticesStart;
            buffer->Write(0, vertexCount * sizeof(FontVertex), verticesStart);

            m_graphics->BindTexture(0, textureHandle);
            m_graphics->DrawTriangleArrays(0, vertexCount);
        }
        m_vb_alloc.Reset();
    }

    float Renderer::GetTextWidthAscii(const char *text) const
    {
        float width = 0.0f;
        while (*text)
        {
            const uint32_t c = uint8_t(*text++);
            const Glyph &glyph = m_font.GetGlyph(c);
            width += float(glyph.m_advance);
        }
        return width;
    }

    float Renderer::GetTextWidthAscii(const char *text, size_t charCount) const
    {
        float width = 0.0f;
        for (; *text && charCount; charCount--)
        {
            const uint32_t c = uint8_t(*text++);
            const Glyph &glyph = m_font.GetGlyph(c);
            width += float(glyph.m_advance);
        }
        return width;
    }

    float Renderer::GetFontHeight() const
    { return m_font.GetHeight(); }

    float Renderer::GetFontLineSpacing() const
    { return m_font.GetLineSpacing(); }

    Texture *Renderer::GetFontTexture() const
    {
        TextureHandle texHandle = m_font.GetTexture(0);
        Texture *texture = m_graphics->GetTexture(texHandle);
        return texture;
    }

} // rob
