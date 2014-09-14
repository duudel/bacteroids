
#ifndef H_ROB_SHADER_PROGRAM_H
#define H_ROB_SHADER_PROGRAM_H

#include "GLTypes.h"
#include "../Types.h"

namespace rob
{

    class VertexShader;
    class FragmentShader;

    class ShaderProgram
    {
    public:
        ShaderProgram();
        ~ShaderProgram();

        GLuint GetObject() const;

        void SetShaders(VertexShader *vertexShader, FragmentShader *fragmentShader);
        bool Link();

        bool IsLinked() const;

        size_t GetLinkInfoSize() const;
        void GetLinkInfo(char *buffer, size_t bufferSize) const;

    private:
        GLuint m_object;
        bool m_linked;
    };

} // rob

#endif // H_ROB_SHADER_PROGRAM_H

