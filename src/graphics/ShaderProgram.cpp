
#include "ShaderProgram.h"
#include "Shader.h"

#include <GL/glew.h>

namespace rob
{

    ShaderProgram::ShaderProgram()
        : m_object()
        , m_linked(false)
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

} // rob
