#version 460

in vec2 tex_coords;

out vec4 frag_color;

#include "Common_Global.glsl"

uniform sampler2D depth_texture;
// Add more textures for different views

float linearlize_depth(float depth)
{
    float n = page_data.depth_settings.r;
    float f = page_data.depth_settings.g;

    float z = depth * 2.0 - 1.0;
    return (2.0 * n * f) / (f + n - z * (f - n));
}

void main()
{
    switch(u_DebugMode)
    {
        case 0: // Standard Color View
            // NOTE: Ideally it's best to not make another FBO pass if it's going to look the same as before,
            //       so this might not even happen. It's mostly to align the selection indices with the switch case.
            frag_color = texture(color_texture, tex_coords);
            break;

        case 1: // Linear Depth View
            float raw_depth = texture(depth_texture, tex_coords).r;
            float linear_depth = linearlize_depth(raw_depth);
            float normalized_depth = linear_depth / page_data.depth_settings.g;
            frag_color = vec4(vec3(normalized_depth), 1.0);
            break;

        default:
            frag_color = texture(color_texture, tex_coords);
    }
}
