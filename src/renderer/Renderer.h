
#ifndef H_ROB_RENDERER_H
#define H_ROB_RENDERER_H

#include "../graphics/GraphicsTypes.h"
#include "../resource/ResourceID.h"
#include "Color.h"

#include "../math/Types.h"

namespace rob
{

    class Graphics;
    class MasterCache;
    class LinearAllocator;

    struct GlobalUniforms
    {
        UniformHandle projection;
        UniformHandle position;
        UniformHandle time;
    };

    class Renderer
    {
    public:
        Renderer(Graphics *graphics, LinearAllocator &alloc);
        Renderer(const Renderer&) = delete;
        Renderer& operator = (const Renderer&) = delete;
        ~Renderer();

        ShaderProgramHandle CompileShaderProgram(const char * const vert, const char * const frag);

        Graphics* GetGraphics();
        const GlobalUniforms& GetGlobals() const;

        void GetScreenSize(int *screenW, int *screenH) const;
        void SetProjection(const mat4f &projection);
        void SetTime(float time);

        void BindShader(ShaderProgramHandle shader);
        void BindColorShader();

        void SetColor(const Color &color);
        void DrawRectangle(float x0, float y0, float x1, float y1);
        void DrawFilledRectangle(float x0, float y0, float x1, float y1);
        void DrawCirlce(float x, float y, float radius);
        void DrawFilledCirlce(float x, float y, float radius);
        void DrawFilledCirlce(float x, float y, float radius, const Color &center);

    private:
        LinearAllocator &m_alloc;
        Graphics *m_graphics;

        GlobalUniforms m_globals;

        VertexBufferHandle      m_vertexBuffer;
        ShaderProgramHandle     m_shaderProgram;
        ShaderProgramHandle     m_colorProgram;

        Color m_color;
    };

} // rob

#endif // H_ROB_RENDERER_H

