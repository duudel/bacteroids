
#ifndef H_ROB_GRAPHICS_TYPES_H
#define H_ROB_GRAPHICS_TYPES_H

namespace rob
{

    class Texture;
    class VertexBuffer;
    class IndexBuffer;
    class VertexShader;
    class FragmentShader;
    class ShaderProgram;

    typedef unsigned int GraphicsHandle;

    typedef GraphicsHandle TextureHandle;
    typedef GraphicsHandle VertexBufferHandle;
    typedef GraphicsHandle IndexBufferHandle;
    typedef GraphicsHandle VertexShaderHandle;
    typedef GraphicsHandle FragmentShaderHandle;
    typedef GraphicsHandle ShaderProgramHandle;

    static const GraphicsHandle InvalidHandle = ~0;

} // rob

#endif // H_ROB_GRAPHICS_TYPES_H

