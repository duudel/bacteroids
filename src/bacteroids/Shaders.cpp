
#include "Shaders.h"

#define GLSL(x) "#version 120\n" #x

namespace bact
{

const ShaderDef g_playerShader = {
     // Vertex shader
    GLSL(
        uniform mat4 u_projection;
        uniform vec4 u_position;
        uniform vec4 u_velocity;
        uniform int u_time_ms;
        attribute vec2 a_position;
        attribute vec4 a_color;
        varying vec4 v_color;
        void main()
        {
            const int T_resolution = 1000;
            const int WrapPeriod = int(T_resolution*2*3.14159);

            int time = u_time_ms;
            float t = mod(time * 16.0, WrapPeriod) / float(T_resolution);

            float scale = sin(t) * 0.5;
            vec2 pos = a_position;
            vec2 vel = u_velocity.xy;
            vec2 offset = vel * dot(pos, vel * scale) * 0.05;
            pos += u_position.xy + offset;
            gl_Position = u_projection * vec4(pos, 0.0, 1.0);
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

const ShaderDef g_bacterShader = {
     // Vertex shader
    GLSL(
        uniform mat4 u_projection;
        uniform vec4 u_position;
        uniform int u_time_ms;
        uniform float u_anim;
        attribute vec2 a_position;
        attribute vec4 a_color;
        varying vec4 v_color;
        varying float v_dist;
        void main()
        {
            const int T_resolution = 1000;
            const int WrapPeriod = int(T_resolution*2*3.14159);

            int time = u_time_ms;

            vec2 pos = a_position;
            float radius = length(pos);
            float dist = 0.0;
            if (radius > 0.01)
            {
                float t1 = mod(time * 12, WrapPeriod) / float(T_resolution);
                float t2 = mod(time * 10, WrapPeriod) / float(T_resolution);

                float a = atan(pos.x, pos.y);
                float phase1 = u_anim + a*11.0;
                float phase2 = u_anim + a*5.0;

                float w = sin(phase1 + t1) * 2.3;
                float u = sin(phase2 - t2) * 3.0;

                float r = (1.0 + (w + u)/65.0) * radius;
                pos = normalize(pos) * r;
                dist = 1.0;
            }
            gl_Position = u_projection * vec4(pos + u_position.xy, 0.0, 1.0);
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

const ShaderDef g_projectileShader = {
     // Vertex shader
    GLSL(
        uniform mat4 u_projection;
        uniform vec4 u_position;
        uniform vec4 u_velocity;
        attribute vec2 a_position;
        attribute vec4 a_color;
        varying vec4 v_color;
        void main()
        {
            vec2 pos = a_position;
            vec2 vel = u_velocity.xy;
            vec2 offset = vel * dot(pos, vel) * 0.005;
            pos += u_position.xy + offset;
            gl_Position = u_projection * vec4(pos, 0.0, 1.0);
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

const ShaderDef g_fontShader = {
     // Vertex shader
    GLSL(
        uniform mat4 u_projection;
        uniform vec4 u_position;
        attribute vec4 a_position;
        attribute vec4 a_color;
        varying vec2 v_uv;
        varying vec4 v_color;
        void main()
        {
            float scale = (1.0 / 40.0);

            vec2 pos = a_position.xy - u_position.xy;
            pos.y = -pos.y;
            pos = u_position.xy + pos * scale;
            gl_Position = u_projection * vec4(pos, 0.0, 1.0);
            v_uv = a_position.zw;
            v_color = a_color;
        }
    ),

    // Fragment shader
    GLSL(
        uniform sampler2D u_texture0;
        varying vec2 v_uv;
        varying vec4 v_color;
        void main()
        {
            vec4 color = texture2D(u_texture0, v_uv);
            gl_FragColor = v_color * color;
        }
    )
};

} // bact
