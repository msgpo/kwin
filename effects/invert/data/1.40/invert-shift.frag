#version 140

uniform sampler2D sampler;
uniform vec4 modulation;
uniform float saturation;

in vec2 texcoord0;

out vec4 fragColor;

void main()
{
    vec4 tex = texture(sampler, texcoord0);

    if (saturation != 1.0) {
        vec3 desaturated = tex.rgb * vec3(0.30, 0.59, 0.11);
        desaturated = vec3(dot(desaturated, tex.rgb));
        tex.rgb = tex.rgb * vec3(saturation) + desaturated * vec3(1.0 - saturation);
    }

    float delta = tex.a - min(tex.r, min(tex.g, tex.b)) - max(tex.r, max(tex.g, tex.b));
    tex.rgb += vec3(delta);

    tex *= modulation;
    tex.rgb *= tex.a;

    fragColor = tex;
}
