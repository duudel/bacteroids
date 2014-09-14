
#ifndef H_ROB_RENDERER_H
#define H_ROB_RENDERER_H

#include "../graphics/GraphicsTypes.h"
#include "../resource/ResourceID.h"
#include "Color.h"

namespace rob
{

    class Graphics;
    class MasterCache;
    class LinearAllocator;

    class Renderer
    {
    public:
        Renderer(Graphics *graphics, LinearAllocator &alloc);
        ~Renderer();

        void GetScreenSize(int *screenW, int *screenH) const;

        void SetColor(const Color &color);
        void DrawRectangle(float x0, float y0, float x1, float y1);
        void DrawFilledRectangle(float x0, float y0, float x1, float y1);
        void DrawCirlce(float x, float y, float radius);
        void DrawFilledCirlce(float x, float y, float radius);
        void DrawFilledCirlce(float x, float y, float radius, const Color &center);

    private:
        LinearAllocator &m_alloc;
        Graphics *m_graphics;

        VertexShaderHandle      m_vertexShader;
        FragmentShaderHandle    m_fragmentShader;
        ShaderProgramHandle     m_shaderProgram;

        VertexBufferHandle      m_vertexBuffer;

        VertexShaderHandle      m_colorVertexShader;
        FragmentShaderHandle    m_colorFragmentShader;
        ShaderProgramHandle     m_colorProgram;
        Color m_color;
    };

} // rob

#endif // H_ROB_RENDERER_H

