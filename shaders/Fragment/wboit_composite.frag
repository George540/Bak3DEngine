#version 460 core

in vec2 TextCoords;

out vec4 frag_color;

uniform sampler2D accumulation_texture; // 0
uniform sampler2D revealage_texture;    // 1

void main()
{
    float reveal = texture(revealage_texture, TextCoords).r;

    // Fully uncovered pixel. Skip compositing.
    if (reveal >= 0.999)
    {
        discard;
    }

    vec4 accum = texture(accumulation_texture, TextCoords);

    // Guard divide-by-zero when accumulated weight is ~0.
    vec3 average_color = accum.rgb / max(accum.a, 1e-5);

    frag_color = vec4(average_color, 1.0 - reveal);
}
