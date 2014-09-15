
#ifndef H_BACT_SHADERS_H
#define H_BACT_SHADERS_H

namespace bact
{

    struct ShaderDef
    {
        const char * const m_vertexShader;
        const char * const m_fragmentShader;
    };

    extern const ShaderDef g_bacterShader;

} // bact

#endif // H_BACT_SHADERS_H

