
#include "Graphics.h"

#include "Texture.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "ShaderProgram.h"
#include "Uniform.h"

#include "../memory/LinearAllocator.h"

#include "../Log.h"

#include "GLCheck.h"
#include <GL/glew.h>

namespace rob
{

    Graphics::Graphics(LinearAllocator &alloc)
        : m_initialized(false)
    {
        if (::glewInit() != GLEW_OK)
        {
            log::Error("Could not initialize GLEW");
            return;
        }

        SetViewport(0, 0, 0, 0);

        InitState();
        ::glEnable(GL_BLEND);
        ::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        const size_t blockSize = 1024;
        m_textures.SetMemory(alloc.Allocate(blockSize), blockSize);
        m_vertexBuffers.SetMemory(alloc.Allocate(blockSize), blockSize);
        m_indexBuffers.SetMemory(alloc.Allocate(blockSize), blockSize);
        m_vertexShaders.SetMemory(alloc.Allocate(blockSize), blockSize);
        m_fragmentShaders.SetMemory(alloc.Allocate(blockSize), blockSize);
        m_shaderPrograms.SetMemory(alloc.Allocate(blockSize), blockSize);
        m_uniforms.SetMemory(alloc.Allocate(blockSize), blockSize);

        m_initialized = true;
    }

    void Graphics::InitState()
    {
        for (size_t i = 0; i < MAX_TEXTURE_UNITS; i++)
            m_bind.texture[i] = InvalidHandle;
        m_bind.vertexBuffer = InvalidHandle;
        m_bind.indexBuffer = InvalidHandle;
        m_bind.shaderProgram = InvalidHandle;
        m_state = m_bind;
    }

    Graphics::~Graphics()
    { }

    bool Graphics::IsInitialized() const
    { return m_initialized; }

    void Graphics::SetViewport(int x, int y, int w, int h)
    {
        ::glViewport(x, y, w, h);
        m_viewport.x = x;
        m_viewport.y = y;
        m_viewport.w = w;
        m_viewport.h = h;
    }

    void Graphics::GetViewport(int *x, int *y, int *w, int *h) const
    {
        *x = m_viewport.x;
        *y = m_viewport.y;
        *w = m_viewport.w;
        *h = m_viewport.h;
    }

    void Graphics::Clear()
    { ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

    void Graphics::SetClearColor(float r, float g, float b)
    { ::glClearColor(r, g, b, 1.0f); }


    void Graphics::SetTexture(size_t unit, TextureHandle texture)
    {
        ROB_ASSERT(unit < MAX_TEXTURE_UNITS);
        m_state.texture[unit] = texture;
    }

    void Graphics::SetVertexBuffer(VertexBufferHandle buffer)
    { m_state.vertexBuffer = buffer; }

    void Graphics::SetIndexBuffer(IndexBufferHandle buffer)
    { m_state.indexBuffer = buffer; }

    void Graphics::SetShaderProgram(ShaderProgramHandle program)
    { m_state.shaderProgram = program; }

    void Graphics::UpdateState()
    {
        for (size_t i = 0; i < MAX_TEXTURE_UNITS; i++)
        {
            BindTexture(i, m_state.texture[i]);
        }
        BindVertexBuffer(m_state.vertexBuffer);
        BindIndexBuffer(m_state.indexBuffer);
    }

    void Graphics::BindTexture(size_t unit, TextureHandle texture)
    {
        ROB_ASSERT(unit < MAX_TEXTURE_UNITS);

        if (m_bind.texture[unit] == texture)
            return;

        m_bind.texture[unit] = texture;
        ::glActiveTexture(GL_TEXTURE0 + unit);
        GL_CHECK;
        if (texture == InvalidHandle)
        {
            ::glBindTexture(GL_TEXTURE_2D, 0);
            GL_CHECK;
        }
        else
        {
            Texture *tex = m_textures.Get(texture);
            ::glBindTexture(GL_TEXTURE_2D, tex->GetObject());
            GL_CHECK;
        }
    }

    void Graphics::BindVertexBuffer(VertexBufferHandle buffer)
    {
        if (m_bind.vertexBuffer == buffer)
            return;

        m_bind.vertexBuffer = buffer;
        if (buffer == InvalidHandle)
        {
            ::glBindBuffer(GL_ARRAY_BUFFER, 0);
            GL_CHECK;
        }
        else
        {
            VertexBuffer *vb = m_vertexBuffers.Get(buffer);
            ::glBindBuffer(GL_ARRAY_BUFFER, vb->GetObject());
            GL_CHECK;
        }
    }

    void Graphics::BindIndexBuffer(IndexBufferHandle buffer)
    {
        if (m_bind.indexBuffer == buffer)
            return;

        m_bind.indexBuffer = buffer;
        if (buffer == InvalidHandle)
        {
            ::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            GL_CHECK;
        }
        else
        {
            IndexBuffer *ib = m_indexBuffers.Get(buffer);
            ::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib->GetObject());
            GL_CHECK;
        }
    }

    void Graphics::BindShaderProgram(ShaderProgramHandle program)
    {
        ShaderProgram *p = (program != InvalidHandle) ?
            m_shaderPrograms.Get(program) : nullptr;

        if (m_bind.shaderProgram != program)
        {
            m_bind.shaderProgram = program;
            if (program == InvalidHandle)
            {
                ::glUseProgram(0);
                GL_CHECK;
            }
            else
            {
                ::glUseProgram(p->GetObject());
                GL_CHECK;
            }
        }
        if (p) p->UpdateUniforms(this);
    }

    void Graphics::SetUniform(UniformHandle u, int value)
    {
        Uniform *uniform = GetUniform(u);
        ROB_ASSERT(uniform->m_type == UniformType::Int);
        uniform->SetValue(value);
        if (m_bind.shaderProgram != InvalidHandle)
        {
            ShaderProgram *p = m_shaderPrograms.Get(m_bind.shaderProgram);
            p->UpdateUniforms(this);
        }
    }

    void Graphics::SetUniform(UniformHandle u, float value)
    {
        Uniform *uniform = GetUniform(u);
        ROB_ASSERT(uniform->m_type == UniformType::Float);
        uniform->SetValue(value);
        if (m_bind.shaderProgram != InvalidHandle)
        {
            ShaderProgram *p = m_shaderPrograms.Get(m_bind.shaderProgram);
            p->UpdateUniforms(this);
        }
    }

    void Graphics::SetUniform(UniformHandle u, const vec4f &value)
    {
        Uniform *uniform = GetUniform(u);
        ROB_ASSERT(uniform->m_type == UniformType::Vec4);
        uniform->SetValue(value);
        if (m_bind.shaderProgram != InvalidHandle)
        {
            ShaderProgram *p = m_shaderPrograms.Get(m_bind.shaderProgram);
            p->UpdateUniforms(this);
        }
    }

    void Graphics::SetUniform(UniformHandle u, const mat4f &value)
    {
        Uniform *uniform = GetUniform(u);
        ROB_ASSERT(uniform->m_type == UniformType::Mat4);
        uniform->SetValue(value);
        if (m_bind.shaderProgram != InvalidHandle)
        {
            ShaderProgram *p = m_shaderPrograms.Get(m_bind.shaderProgram);
            p->UpdateUniforms(this);
        }
    }


    void Graphics::SetAttrib(size_t attr, size_t size, size_t stride, size_t offset)
    {
        ROB_ASSERT(attr < 8);
        ::glEnableVertexAttribArray(attr);
        GL_CHECK;
        ::glVertexAttribPointer(attr, size, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(offset));
        GL_CHECK;
    }


    void Graphics::DrawTriangleArrays(size_t first, size_t count)
    {
        ::glDrawArrays(GL_TRIANGLES, first, count);
        GL_CHECK;
    }

    void Graphics::DrawTriangleStripArrays(size_t first, size_t count)
    {
        ::glDrawArrays(GL_TRIANGLE_STRIP, first, count);
        GL_CHECK;
    }

    void Graphics::DrawTriangleFanArrays(size_t first, size_t count)
    {
        ::glDrawArrays(GL_TRIANGLE_FAN, first, count);
        GL_CHECK;
    }

    void Graphics::DrawLineArrays(size_t first, size_t count)
    {
        ::glDrawArrays(GL_LINES, first, count);
        GL_CHECK;
    }

    void Graphics::DrawLineLoopArrays(size_t first, size_t count)
    {
        ::glDrawArrays(GL_LINE_LOOP, first, count);
        GL_CHECK;
    }

    // Textures

    TextureHandle Graphics::CreateTexture()
    {
        Texture *texture = m_textures.Obtain();
        return m_textures.IndexOf(texture);
    }

    Texture* Graphics::GetTexture(TextureHandle texture)
    { return m_textures.Get(texture); }

    void Graphics::DestroyTexture(TextureHandle texture)
    { m_textures.Return(GetTexture(texture)); }

    // Vertex and index buffers

    VertexBufferHandle Graphics::CreateVertexBuffer()
    {
        VertexBuffer *buffer = m_vertexBuffers.Obtain();
        return m_vertexBuffers.IndexOf(buffer);
    }

    VertexBuffer* Graphics::GetVertexBuffer(VertexBufferHandle buffer)
    { return m_vertexBuffers.Get(buffer); }

    void Graphics::DestroyVertexBuffer(VertexBufferHandle buffer)
    { m_vertexBuffers.Return(GetVertexBuffer(buffer)); }

    IndexBufferHandle Graphics::CreateIndexBuffer()
    {
        IndexBuffer *buffer = m_indexBuffers.Obtain();
        return m_indexBuffers.IndexOf(buffer);
    }

    IndexBuffer* Graphics::GetIndexBuffer(IndexBufferHandle buffer)
    { return m_indexBuffers.Get(buffer); }

    void Graphics::DestroyIndexBuffer(IndexBufferHandle buffer)
    { m_indexBuffers.Return(GetIndexBuffer(buffer)); }

    // Shaders

    VertexShaderHandle Graphics::CreateVertexShader()
    {
        VertexShader *shader = m_vertexShaders.Obtain();
        return m_vertexShaders.IndexOf(shader);
    }

    VertexShader* Graphics::GetVertexShader(VertexShaderHandle shader)
    { return m_vertexShaders.Get(shader); }

    void Graphics::DestroyVertexShader(VertexShaderHandle shader)
    { m_vertexShaders.Return(GetVertexShader(shader)); }

    FragmentShaderHandle Graphics::CreateFragmentShader()
    {
        FragmentShader *shader = m_fragmentShaders.Obtain();
        return m_fragmentShaders.IndexOf(shader);
    }

    FragmentShader* Graphics::GetFragmentShader(FragmentShaderHandle shader)
    { return m_fragmentShaders.Get(shader); }

    void Graphics::DestroyFragmentShader(FragmentShaderHandle shader)
    { m_fragmentShaders.Return(GetFragmentShader(shader)); }

    ShaderProgramHandle Graphics::CreateShaderProgram()
    {
        ShaderProgram *program = m_shaderPrograms.Obtain();
        return m_shaderPrograms.IndexOf(program);
    }

    ShaderProgram* Graphics::GetShaderProgram(ShaderProgramHandle program)
    { return m_shaderPrograms.Get(program); }

    void Graphics::DestroyShaderProgram(ShaderProgramHandle program)
    { m_shaderPrograms.Return(GetShaderProgram(program)); }




    template <size_t N>
    static void CopyToFixedString(char (&dest)[N], const char *src)
    {
        size_t i = 0;
        for (; i < N && *src; i++, src++)
            dest[i] = *src;
        dest[i] = 0;
    }

    // Uniforms

    UniformHandle Graphics::CreateUniform(const char *name, UniformType type)
    {
        Uniform *uniform = m_uniforms.Obtain();
        uniform->m_type         = type;
        uniform->m_generation   = 0;
        uniform->m_upload       = Uniform::GetUploadFuncFromType(type);
        CopyToFixedString(uniform->m_name, name);
        return m_uniforms.IndexOf(uniform);
    }

    Uniform* Graphics::GetUniform(UniformHandle uniform)
    { return m_uniforms.Get(uniform); }

    void Graphics::DestroyUniform(UniformHandle uniform)
    { m_uniforms.Return(GetUniform(uniform)); }

    void Graphics::AddProgramUniform(ShaderProgramHandle program, UniformHandle uniform)
    {
        ShaderProgram *p = GetShaderProgram(program);
        Uniform *u = GetUniform(uniform);
        p->AddUniform(uniform, u->m_name);
    }

} // rob
