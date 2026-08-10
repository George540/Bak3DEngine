#version 460 core

layout(location = 0) out vec4 accumulation_color;
layout(location = 1) out float revealage;

in VS_OUT
{
    vec3 frag_position;
} fs_in;

//@TODO: Add to particle config UBO
uniform vec3 particle_albedo = vec3(1.0);
uniform float scatter_power = 10.0;
uniform float edge_softness = 0.7;

#include "Common_Global.glsl"

float calculate_wboit_weight(float alpha)
{
    // Weight function: https://learnopengl.com/Guest-Articles/2020/OIT/Weighted-Blended
    return clamp(pow(min(1.0, alpha * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);
}

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

    float weight = calculate_wboit_weight(alpha);

    accumulation_color = vec4(color * alpha, alpha) * weight;
    revealage = alpha;
}
