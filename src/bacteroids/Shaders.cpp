
#include "Shaders.h"

#define GLSL(x) "#version 120\n" #x

namespace bact
{

const ShaderDef g_bacterShader = {
     // Vertex shader
    GLSL(
        uniform mat4 u_projection;
        uniform vec4 u_position;
        uniform float u_time;
        attribute vec2 a_position;
        attribute vec4 a_color;
        varying vec4 v_color;
        varying float v_dist;
        void main()
        {
            vec2 pos = a_position - u_position.xy;
            vec2 offset = vec2(0.0);
            float len = length(pos);
            float dist = 0.0;
            if (len > 0.01)
            {
//                float t = (u_time + u_position.x + u_position.y) * 10;
                float t = u_time * 10;
                float a = atan(pos.x, pos.y);
                float w = sin(t*1.2 + a*11) * 2.3;
                float u = sin(-t*1.0 + a*5) * 3;
                float r = (w + u)/65.0 * len;
                offset = normalize(pos) * r;
                dist = 1.0;
            }
            gl_Position = u_projection * vec4(a_position + offset, 0.0, 1.0);
            v_color = a_color;
            v_dist = dist;
        }
    ),

    // Fragment shader
    GLSL(
        varying vec4 v_color;
        varying float v_dist;
        void main()
        {
            float d = v_dist;
            d = 0.08 + smoothstep(0.0, 0.7, d) * 0.42 + 0.5 * smoothstep(0.75, 1.0, d);
//            gl_FragColor = vec4(v_color.rgb * d, 0.4 + d);
            gl_FragColor = vec4(v_color.rgb * d, d);
        }
    )
};

} // bact
