#version 460 core

in vec2 TexCoords;
in vec4 ParticleColor;

out vec4 color;

uniform sampler2D sprite;

#include "Common_Global.glsl"

void main()
{
    vec4 tex_color = texture(sprite, TexCoords);

    // Discard fully transparent fragments (also catches GPU-culled instances)
    if (tex_color.a * ParticleColor.a < 0.01)
    {
        discard;
    }

    color = tex_color * ParticleColor;
}