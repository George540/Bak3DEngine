#version 460

in vec2 TextCoords;

out vec4 frag_color;

#include "Common_Global.glsl"

layout(binding = 0) uniform sampler2D debug_view_texture;
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
    vec4 texture_sample = texture(debug_view_texture, TextCoords);

    switch (page_data.debug_mode)
    {
        case DEBUG_VIEW_GBUFFER_POSITION:
        {
            frag_color = vec4(texture_sample.rgb, 1.0);
            break;
        }
        case DEBUG_VIEW_GBUFFER_ALBEDO:
        {
            frag_color = vec4(texture_sample.rgb, 1.0);
            break;
        }
        case DEBUG_VIEW_GBUFFER_NORMALS:
        {
            frag_color = vec4(texture_sample.rgb, 1.0);
            break;
        }
        case DEBUG_VIEW_GBUFFER_SPECULAR:
        {
            frag_color = vec4(vec3(texture_sample.a), 1.0);
            break;
        }
        case DEBUG_VIEW_DEPTH:
        {
            float raw_depth = texture_sample.r;
            float linear_depth = linearize_depth(raw_depth);
            float normalized_depth = linear_depth / page_data.depth_settings.g;
            frag_color = vec4(vec3(normalized_depth), 1.0);
            break;
        }
        default:
            // Unhandled mode. Shouldn't be reached. Easy to spot with magenta color
            frag_color = vec4(1.0, 0.0, 1.0, 1.0);
    }
}
