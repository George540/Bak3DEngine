#version 460

in vec2 TextCoords;

out vec4 frag_color;

#include "Common_Global.glsl"

uniform sampler2D depth_texture;
uniform sampler2D ao_texture;
// Add more textures for different views

float linearize_depth(float depth)
{
    float n = page_data.depth_settings.r;
    float f = page_data.depth_settings.g;

    float z = depth * 2.0 - 1.0;
    return (2.0 * n * f) / (f + n - z * (f - n));
}

void main()
{
    switch (page_data.debug_mode)
    {
        case DEBUG_VIEW_DEPTH:
        {
            float raw_depth = texture(depth_texture, TextCoords).r;
            float linear_depth = linearize_depth(raw_depth);
            float normalized_depth = linear_depth / page_data.depth_settings.g;
            frag_color = vec4(vec3(normalized_depth), 1.0);
            break;
        }
        // case DEBUG_VIEW_AO:
        // {
        //     float ao = texture(ao_texture, TextCoords).r;
        //     frag_color = vec4(vec3(ao), 1.0);
        //     break;
        // }
        default:
            // Unhandled mode. Shouldn't be reached. Easy to spot with magenta color
            frag_color = vec4(1.0, 0.0, 1.0, 1.0);
    }
}
