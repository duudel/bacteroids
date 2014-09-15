
#include "Renderer.h"
#include "../graphics/Graphics.h"
#include "../graphics/Shader.h"
#include "../graphics/ShaderProgram.h"
#include "../graphics/VertexBuffer.h"

#include "../memory/LinearAllocator.h"

#include "../math/Math.h"

#include "../Log.h"

#include <GL/glew.h>

namespace rob
{

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
//            vec4 color = vec4(v_uv, 0.0, 1.0);
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

    UniformHandle g_u_projection = InvalidHandle;

    void CompileShaderProgram(Graphics *graphics, const char * const vert, const char * const frag,
                              VertexShaderHandle &vs, FragmentShaderHandle &fs, ShaderProgramHandle &p)
    {
        vs = graphics->CreateVertexShader();
        fs = graphics->CreateFragmentShader();
        p = graphics->CreateShaderProgram();

        VertexShader *vertShader = graphics->GetVertexShader(vs);
        vertShader->SetSource(vert);
        if (!vertShader->Compile())
        {
            char buffer[512];
            vertShader->GetCompileInfo(buffer, 512);
            log::Error(&buffer[0]);
            return;
        }

        FragmentShader *fragShader = graphics->GetFragmentShader(fs);
        fragShader->SetSource(frag);
        if (!fragShader->Compile())
        {
            char buffer[512];
            fragShader->GetCompileInfo(buffer, 512);
            log::Error(&buffer[0]);
            return;
        }

        ShaderProgram *program = graphics->GetShaderProgram(p);
        program->SetShaders(vertShader, fragShader);
        if (!program->Link())
        {
            char buffer[512];
            program->GetLinkInfo(buffer, 512);
            log::Error(&buffer[0]);
            return;
        }

        graphics->AddProgramUniform(p, g_u_projection);

//        GLint loc = ::glGetUniformLocation(program->GetObject(), "u_texture");
//        graphics->BindShaderProgram(p);
//        ::glUniform1i(loc, 0);
    }


    Renderer::Renderer(Graphics *graphics, LinearAllocator &alloc)
        : m_alloc(alloc)
        , m_graphics(graphics)
        , m_vertexShader(InvalidHandle)
        , m_fragmentShader(InvalidHandle)
        , m_shaderProgram(InvalidHandle)
        , m_vertexBuffer(InvalidHandle)
        , m_colorVertexShader(InvalidHandle)
        , m_colorFragmentShader(InvalidHandle)
        , m_colorProgram(InvalidHandle)
    {
        g_u_projection = m_graphics->CreateUniform("u_projection", UniformType::Mat4);
        m_graphics->SetUniform(g_u_projection, mat4f::Identity);

        CompileShaderProgram(m_graphics, g_vertexShader, g_fragmentShader,
                             m_vertexShader, m_fragmentShader, m_shaderProgram);
        CompileShaderProgram(m_graphics, g_colorVertexShader, g_colorFragmentShader,
                             m_colorVertexShader, m_colorFragmentShader, m_colorProgram);


        m_vertexBuffer = m_graphics->CreateVertexBuffer();
        m_graphics->BindVertexBuffer(m_vertexBuffer);
        VertexBuffer *vb = m_graphics->GetVertexBuffer(m_vertexBuffer);
        vb->Resize(4 * 1024 * 1024, false);
    }

    Renderer::~Renderer()
    {
        m_graphics->DestroyShaderProgram(m_shaderProgram);
        m_graphics->DestroyVertexShader(m_vertexShader);
        m_graphics->DestroyFragmentShader(m_fragmentShader);
        m_graphics->DestroyVertexBuffer(m_vertexBuffer);

        m_graphics->DestroyShaderProgram(m_colorProgram);
        m_graphics->DestroyVertexShader(m_colorVertexShader);
        m_graphics->DestroyFragmentShader(m_colorFragmentShader);
    }

    void Renderer::GetScreenSize(int *screenW, int *screenH) const
    {
        int x, y, w, h;
        m_graphics->GetViewport(&x, &y, &w, &h);
        *screenW = w;
        *screenH = h;
    }

    void Renderer::SetColor(const Color &color)
    { m_color = color; }

    struct ColorVertex
    {
        float x, y;
        float r, g, b, a;
    };

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
        m_graphics->BindShaderProgram(m_colorProgram);
        m_graphics->DrawTriangleStripArrays(0, 4);
    }

    void Renderer::DrawCirlce(float x, float y, float radius)
    {
        const size_t segments = 24;
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

        m_graphics->BindVertexBuffer(m_vertexBuffer);
        VertexBuffer *buffer = m_graphics->GetVertexBuffer(m_vertexBuffer);
        buffer->Write(0, sizeof(vertices), vertices);
        m_graphics->SetAttrib(0, 2, sizeof(ColorVertex), 0);
        m_graphics->SetAttrib(1, 4, sizeof(ColorVertex), sizeof(float) * 2);
        m_graphics->BindShaderProgram(m_colorProgram);
        m_graphics->DrawLineLoopArrays(0, vertexCount);
    }

    void Renderer::DrawFilledCirlce(float x, float y, float radius)
    {
        const size_t segments = 24;
        const size_t quarter = segments / 4;
        const size_t vertexCount = segments + 1;
        ColorVertex vertices[vertexCount];

        float angle = 0.0f;
        const float deltaAngle = 2.0f * PI_f / segments;
        vertices[0] = { x - radius, y, m_color.r, m_color.g, m_color.b, m_color.a };
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

        m_graphics->BindVertexBuffer(m_vertexBuffer);
        VertexBuffer *buffer = m_graphics->GetVertexBuffer(m_vertexBuffer);
        buffer->Write(0, sizeof(vertices), vertices);
        m_graphics->SetAttrib(0, 2, sizeof(ColorVertex), 0);
        m_graphics->SetAttrib(1, 4, sizeof(ColorVertex), sizeof(float) * 2);
        m_graphics->BindShaderProgram(m_colorProgram);
        m_graphics->DrawTriangleFanArrays(0, vertexCount);
    }

    void Renderer::DrawFilledCirlce(float x, float y, float radius, const Color &center)
    {
        const size_t segments = 24;
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

        m_graphics->BindVertexBuffer(m_vertexBuffer);
        VertexBuffer *buffer = m_graphics->GetVertexBuffer(m_vertexBuffer);
        buffer->Write(0, sizeof(vertices), vertices);
        m_graphics->SetAttrib(0, 2, sizeof(ColorVertex), 0);
        m_graphics->SetAttrib(1, 4, sizeof(ColorVertex), sizeof(float) * 2);
        m_graphics->BindShaderProgram(m_colorProgram);
        m_graphics->DrawTriangleFanArrays(0, vertexCount);
    }

} // rob
