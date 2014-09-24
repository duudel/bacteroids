
#define GLSL(x) "#version 120\n" #x

namespace rob
{

    extern const char * const g_colorVertexShader = GLSL(
        uniform mat4 u_projection;
        uniform vec4 u_position;
        attribute vec2 a_position;
        attribute vec4 a_color;
        varying vec4 v_color;
        void main()
        {
            vec2 pos = u_position.xy + a_position;
            gl_Position = u_projection * vec4(pos, 0.0, 1.0);
            v_color = a_color;
        }
    );

    extern const char * const g_colorFragmentShader = GLSL(
        varying vec4 v_color;
        void main()
        {
            gl_FragColor = v_color;
        }
    );

    extern const char * const g_fontVertexShader = GLSL(
        uniform mat4 u_projection;
        attribute vec4 a_position;
        attribute vec4 a_color;
        varying vec2 v_uv;
        varying vec4 v_color;
        void main()
        {
            gl_Position = u_projection * vec4(a_position.xy * 1.0, 0.0, 1.0);
            v_uv = a_position.zw;
            v_color = a_color;
        }
    );

    extern const char * const g_fontFragmentShader = GLSL(
        uniform sampler2D u_texture0;
        varying vec2 v_uv;
        varying vec4 v_color;
        void main()
        {
            vec4 color = texture2D(u_texture0, v_uv);
//            gl_FragColor = v_color * color;
            gl_FragColor = vec4(1.0, 1.0, 1.0, color.a);

//            float dist = texture2D(u_texture0, v_uv).r;
//            vec4 color = v_color;
//            float alpha = smoothstep(0.48, 0.5, dist);
//            gl_FragColor = vec4(color.rgb * alpha, alpha * color.a);

//            float alpha = smoothstep(0.46, 0.5, dist);
//            vec4 c = mix(vec4(1.0, 0.0, 1.0, 1.0), color, smoothstep(0.46, 0.52, dist));
//            gl_FragColor = vec4(c.rgb * alpha, alpha * color.a);
        }
    );

} // rob

#undef GLSL
