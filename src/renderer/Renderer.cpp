
#include "Renderer.h"
#include "../graphics/Graphics.h"
#include "../graphics/Shader.h"
#include "../graphics/ShaderProgram.h"
#include "../graphics/VertexBuffer.h"
#include "Font.h"

#include "../math/Math.h"

#include "../Log.h"

#include <GL/glew.h>
#include <cstring>

namespace rob
{

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

    #define GLSL(x) "#version 120\n" #x

    static const char * const g_vertexShader = GLSL(
        uniform mat4 u_projection;
        attribute vec4 a_tile;
        varying vec2 v_uv;
        void main()
        {
            gl_Position = vec4(a_tile.xy, 0.0, 1.0);
            v_uv = a_tile.zw;
        }
    );

    static const char * const g_fragmentShader = GLSL(
        uniform sampler2D u_texture;
        varying vec2 v_uv;
        void main()
        {
            vec4 color = texture2D(u_texture, v_uv);
            gl_FragColor = color;
        }
    );

    static const char * const g_colorVertexShader = GLSL(
        uniform mat4 u_projection;
        attribute vec2 a_position;
        attribute vec4 a_color;
        varying vec4 v_color;
        void main()
        {
            gl_Position = u_projection * vec4(a_position, 0.0, 1.0);
            v_color = a_color;
        }
    );

    static const char * const g_colorFragmentShader = GLSL(
        varying vec4 v_color;
        void main()
        {
            gl_FragColor = v_color;
        }
    );


    ShaderProgramHandle Renderer::CompileShaderProgram(const char * const vert, const char * const frag)
    {
        VertexShaderHandle vs = m_graphics->CreateVertexShader();
        FragmentShaderHandle fs = m_graphics->CreateFragmentShader();
        ShaderProgramHandle p = m_graphics->CreateShaderProgram();

        VertexShader *vertShader = m_graphics->GetVertexShader(vs);
        vertShader->SetSource(vert);
        if (!vertShader->Compile())
        {
            char buffer[512];
            vertShader->GetCompileInfo(buffer, 512);
            log::Error(&buffer[0]);
            return InvalidHandle;
        }

        FragmentShader *fragShader = m_graphics->GetFragmentShader(fs);
        fragShader->SetSource(frag);
        if (!fragShader->Compile())
        {
            char buffer[512];
            fragShader->GetCompileInfo(buffer, 512);
            log::Error(&buffer[0]);
            return InvalidHandle;
        }

        ShaderProgram *program = m_graphics->GetShaderProgram(p);
        program->SetShaders(vertShader, fragShader);
        if (!program->Link())
        {
            char buffer[512];
            program->GetLinkInfo(buffer, 512);
            log::Error(&buffer[0]);
            return InvalidHandle;
        }

        m_graphics->DestroyVertexShader(vs);
        m_graphics->DestroyFragmentShader(fs);

        m_graphics->AddProgramUniform(p, m_globals.projection);
        m_graphics->AddProgramUniform(p, m_globals.position);
        m_graphics->AddProgramUniform(p, m_globals.time);
        return p;
    }

    static const size_t MAX_VERTEX_BUFFER_SIZE = 4 * 1024 * 1024;

    Renderer::Renderer(Graphics *graphics, LinearAllocator &alloc)
        : m_vb_alloc(alloc.Allocate(MAX_VERTEX_BUFFER_SIZE), MAX_VERTEX_BUFFER_SIZE)
        , m_graphics(graphics)
        , m_vertexBuffer(InvalidHandle)
        , m_shaderProgram(InvalidHandle)
        , m_colorProgram(InvalidHandle)
        , m_color()
    {
        m_globals.projection = m_graphics->CreateUniform("u_projection", UniformType::Mat4);
        m_globals.position = m_graphics->CreateUniform("u_position", UniformType::Vec4);
        m_globals.time = m_graphics->CreateUniform("u_time", UniformType::Float);
        m_graphics->SetUniform(m_globals.projection, mat4f::Identity);
        m_graphics->SetUniform(m_globals.time, 0.0f);

        m_shaderProgram = CompileShaderProgram(g_vertexShader, g_fragmentShader);
        m_colorProgram = CompileShaderProgram(g_colorVertexShader, g_colorFragmentShader);

        m_vertexBuffer = m_graphics->CreateVertexBuffer();
        m_graphics->BindVertexBuffer(m_vertexBuffer);
        VertexBuffer *vb = m_graphics->GetVertexBuffer(m_vertexBuffer);
        vb->Resize(MAX_VERTEX_BUFFER_SIZE, false);
    }

    Renderer::~Renderer()
    {
        m_graphics->DestroyVertexBuffer(m_vertexBuffer);
        m_graphics->DestroyShaderProgram(m_shaderProgram);
        m_graphics->DestroyShaderProgram(m_colorProgram);
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

    void Renderer::DrawText(float x, float y, const char *text)
    {
        m_graphics->SetUniform(m_globals.position, vec4f(x, y, 0.0f, 1.0f));
        m_graphics->BindVertexBuffer(m_vertexBuffer);
        VertexBuffer *buffer = m_graphics->GetVertexBuffer(m_vertexBuffer);

        const size_t maxVertexCount = std::strlen(text) * 6;
        FontVertex * const verticesStart = m_vb_alloc.AllocateArray<FontVertex>(maxVertexCount);
        while (*text)
        {
            size_t vertexCount = 0;
            FontVertex *vertex = verticesStart;
            for (char c = *text; *text; c = *text++, vertex++)
            {
//                const Glyph &glyph = m_font->GetGlyph(c);
            }

            buffer->Write(0, vertexCount * sizeof(FontVertex), verticesStart);

            m_graphics->SetAttrib(0, 4, sizeof(FontVertex), 0);
            m_graphics->SetAttrib(1, 4, sizeof(FontVertex), sizeof(float) * 4);
            m_graphics->DrawTriangleArrays(0, vertexCount);
        }
        m_vb_alloc.Reset();
    }

} // rob
