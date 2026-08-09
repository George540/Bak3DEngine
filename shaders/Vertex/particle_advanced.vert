#version 460

layout(location = 0) in vec4 in_particle; // xyz = world position, w = world-space size

out VS_OUT
{
    vec3 frag_position;
} vs_out;

#include "Common_Global.glsl"

//@TODO: Add to particle config UBO
uniform float viewport_height;

float point_scale = 0.01; // to be constant

void main()
{
    vec3 world_position = in_particle.xyz;
    vs_out.frag_position = world_position;
    vec4 view_position = camera_data.view * vec4(world_position, 1.0);
    gl_Position = camera_data.projection * view_position;

    float world_size = in_particle.w;
    float pixel_scale = camera_data.projection[1][1] * viewport_height;
    gl_PointSize = world_size * pixel_scale * point_scale / gl_Position.w;
}
