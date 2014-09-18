
#ifndef H_ROB_RENDERER_H
#define H_ROB_RENDERER_H

#include "../graphics/GraphicsTypes.h"
#include "../resource/ResourceID.h"
#include "Color.h"
#include "Font.h"

#include "../math/Types.h"
#include "../memory/LinearAllocator.h"

namespace rob
{

    class Graphics;
    class MasterCache;
    class Font;

    struct GlobalUniforms
    {
        UniformHandle projection;
        UniformHandle position;
        UniformHandle time;
        UniformHandle texture0;
    };

    struct FontVertex;

    class Renderer
    {
    public:
        Renderer(Graphics *graphics, MasterCache *cache, LinearAllocator &alloc);
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
        void BindFontShader();

        void SetColor(const Color &color);
        void DrawRectangle(float x0, float y0, float x1, float y1);
        void DrawFilledRectangle(float x0, float y0, float x1, float y1);
        void DrawCirlce(float x, float y, float radius);
        void DrawFilledCirlce(float x, float y, float radius);
        void DrawFilledCirlce(float x, float y, float radius, const Color &center);

        void DrawText(float x, float y, const char *text);
        float GetTextWidth(const char *text) const;
        float GetTextWidth(const char *text, size_t charCount) const;

        void DrawTextAscii(float x, float y, const char *text);
        float GetTextWidthAscii(const char *text) const;
        float GetTextWidthAscii(const char *text, size_t charCount) const;

        float GetFontHeight() const;
        float GetFontLineSpacing() const;

    private:
        void AddFontVertex(FontVertex *&vertex, const float x, const float y, const float u, const float v);
        void AddFontQuad(FontVertex *&vertex, const Glyph &glyph,
                           float &cursorX, float &cursorY,
                           const size_t textureW, const size_t textureH);


    private:
        LinearAllocator m_alloc;
        LinearAllocator m_vb_alloc;
        Graphics *m_graphics;

        GlobalUniforms m_globals;

        VertexBufferHandle      m_vertexBuffer;
        ShaderProgramHandle     m_colorProgram;
        ShaderProgramHandle     m_fontProgram;

        Color m_color;
        Font m_font;
    };

} // rob

#endif // H_ROB_RENDERER_H

