
#include "ShaderProgram.h"
#include "Shader.h"
#include "Uniform.h"
#include "Graphics.h"

#include "../Assert.h"

#include <GL/glew.h>

namespace rob
{

    ShaderProgram::ShaderProgram()
        : m_object()
        , m_linked(false)
        , m_uniformCount(0)
    { m_object = ::glCreateProgram(); }

    ShaderProgram::~ShaderProgram()
    { ::glDeleteProgram(m_object); }

    GLuint ShaderProgram::GetObject() const
    { return m_object; }

    void ShaderProgram::SetShaders(VertexShader *vertexShader, FragmentShader *fragmentShader)
    {
        if (!vertexShader->IsCompiled() || !fragmentShader->IsCompiled())
            return;

        ::glAttachShader(m_object, vertexShader->GetObject());
        ::glAttachShader(m_object, fragmentShader->GetObject());
    }

    bool ShaderProgram::Link()
    {
        ::glLinkProgram(m_object);
        GLint linked = GL_FALSE;
        ::glGetProgramiv(m_object, GL_LINK_STATUS, &linked);
        return m_linked = (linked == GL_TRUE) ? true : false;
    }

    bool ShaderProgram::IsLinked() const
    { return m_linked; }

    size_t ShaderProgram::GetLinkInfoSize() const
    {
        GLint len = 0;
        ::glGetProgramiv(m_object, GL_INFO_LOG_LENGTH, &len);
        return static_cast<size_t>(len);
    }

    void ShaderProgram::GetLinkInfo(char *buffer, size_t bufferSize) const
    { ::glGetProgramInfoLog(m_object, bufferSize, 0, buffer); }

    void ShaderProgram::AddUniform(UniformHandle handle, const char *name)
    {
        for (size_t i = 0; i < m_uniformCount; i++)
        {
            UniformInfo &info = m_uniforms[i];
            if (info.handle == handle) break;
        }
        ROB_ASSERT(m_uniformCount < MAX_UNIFORMS);
        UniformInfo &info = m_uniforms[m_uniformCount];
        info.handle = handle;
        info.generation = -1;
        info.location = GetLocation(name);
        if (info.location != -1)
            m_uniformCount++;
    }

    void ShaderProgram::UpdateUniforms(Graphics *graphics)
    {
        for (size_t i = 0; i < m_uniformCount; i++)
        {
            UniformInfo &info = m_uniforms[i];
            const Uniform* u = graphics->GetUniform(info.handle);
            if (info.generation != u->m_generation)
            {
                u->m_upload(info.location, &u->m_value);
                info.generation = u->m_generation;
            }
        }
    }

    GLint ShaderProgram::GetLocation(const char *name) const
    { return ::glGetUniformLocation(m_object, name); }

} // rob
