#version 460 core

in vec2 TextCoords;

out vec4 FragColor;

uniform sampler2D screenTexture;

uniform float kernel_matrix[9];
uniform float kernel_intensity;

const float offset = 1.0 / 300.0;

void main()
{
    vec2 offsets[9] = vec2[]
    (
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right
    );

    vec3 sample_tex[9];
    for (int i = 0; i < 9; i++)
    {
        sample_tex[i] = vec3(texture(screenTexture, TextCoords.st + offsets[i]));
    }

    vec3 kernel_color = vec3(0.0);

    for (int i = 0; i < 9; i++)
    {
        kernel_color += sample_tex[i] * kernel_matrix[i];
    }

    vec3 original = sample_tex[4]; // center sample is the original pixel

    vec3 final_color = original;
    final_color = mix(final_color, kernel_color, kernel_intensity);

    FragColor = vec4(final_color, 1.0);
}
