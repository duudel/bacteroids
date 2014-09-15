
#include "Shaders.h"

#define GLSL(x) "#version 120\n" #x

namespace bact
{

const ShaderDef g_bacterShader = {
     // Vertex shader
    GLSL(
        uniform mat4 u_projection;
        uniform float u_time;
        attribute vec2 a_position;
        attribute vec4 a_color;
        varying vec4 v_color;
        void main()
        {
            vec2 pos = a_position;
            vec2 offset = vec2(0.0);
            float len = length(pos);
            if (len > 0.01)
            {
                float t = u_time * 10;
                float a = atan(pos.x, pos.y);
                float w = sin(t*1.5 + a*13) * 2.3;
                float u = sin(-t*1.3 + a*7) * 3;
                float r = (w + u)/65.0 * len;
                offset = normalize(pos) * r;
            }
            gl_Position = u_projection * vec4(pos + offset, 0.0, 1.0);
            v_color = a_color;
        }
    ),

    // Fragment shader
    GLSL(
        varying vec4 v_color;
        void main()
        {
            gl_FragColor = v_color;
        }
    )
};

} // bact
