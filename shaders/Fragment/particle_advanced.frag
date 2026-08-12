#version 460 core

layout(location = 0) out vec4 accumulation_color;
layout(location = 1) out float revealage;

in VS_OUT
{
    vec3 frag_position;
} fs_in;

//@TODO: Add to particle config UBO
uniform vec3 particle_albedo = vec3(1.0);
uniform float scatter_power = 25.0;
uniform float edge_softness = 0.8;

#include "Common_Global.glsl"

void main()
{
    float alpha = calculate_point_alpha_falloff(gl_PointCoord, edge_softness);

    if (alpha <= 0.001)
    {
        discard;
    }

    vec3 view_direction = get_view_direction(fs_in.frag_position);
    vec3 lighting = process_particle_light(fs_in.frag_position, view_direction, scatter_power);
    vec3 color = particle_albedo * lighting;

    // Use view-space linear depth for stable weight computation
    float linear_depth = get_linear_depth(fs_in.frag_position);
    float weight = calculate_wboit_weight(linear_depth, alpha);

    // WBOIT Accumulation
    accumulation_color = vec4(color * alpha, alpha) * weight;

    // Revealage must be transmittance
    revealage = 1.0 - alpha;
}
