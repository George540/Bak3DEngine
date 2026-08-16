#version 460 core

layout(location = 0) out vec4 accumulation_color;
layout(location = 1) out float revealage;

layout(std430, binding = 16) readonly buffer ParticleBuffer { vec4 position[]; }; // XYZ = world-space position, W = world-space particle size (radius units)

flat in int vs_particle_index;

//@TODO: Add to particle config UBO
uniform vec3 particle_albedo = vec3(1.0);
uniform float scatter_power = 25.0;
uniform float edge_softness = 0.6;

#include "Common_Global.glsl"

void main()
{
    float alpha = calculate_point_alpha_falloff(gl_PointCoord, edge_softness);

    if (alpha <= 0.001)
    {
        discard;
    }

    vec3 world_position = position[vs_particle_index].xyz;
    vec3 view_direction = get_view_direction(world_position);
    vec3 lighting = process_particle_light(world_position, view_direction, scatter_power);
    vec3 color = particle_albedo * lighting;

    float linear_depth = get_linear_depth(world_position);
    float weight = calculate_wboit_weight(linear_depth, alpha);

    accumulation_color = vec4(color * alpha, alpha) * weight;
    revealage = alpha;
}
