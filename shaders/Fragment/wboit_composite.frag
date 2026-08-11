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

    // Guard divide-by-zero and guard runaway values from weight-precision blowup.
    // Clamp defensively while you calibrate calculate_wboit_weight() for your scene scale.
    vec3 average_color = clamp(accum.rgb / max(accum.a, 1e-4), vec3(0.0), vec3(1.0));

    frag_color = vec4(average_color, clamp(1.0 - reveal, 0.0, 1.0));
}
