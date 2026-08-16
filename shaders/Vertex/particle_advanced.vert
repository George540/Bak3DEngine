#version 460

layout(std430, binding = 16) readonly buffer ParticleBuffer { vec4 position[]; }; // XYZ = world-space position, W = world-space particle size (radius units)

#include "Common_Global.glsl"

//@TODO: Add to particle config UBO
uniform float viewport_height;

float point_scale = 0.01; // to be constant

flat out int vs_particle_index;

void main()
{
    vs_particle_index = gl_VertexID;
    vec3 world_position = position[gl_VertexID].xyz;
    vec4 view_position = camera_data.view * vec4(world_position, 1.0);
    gl_Position = camera_data.projection * view_position;

    float world_size = position[gl_VertexID].w;
    float pixel_scale = camera_data.projection[1][1] * viewport_height;
    //gl_PointSize = world_size * pixel_scale * point_scale / gl_Position.w; // @TODO: See if this is even necessary. Resizing point clouds to be bigger can be costlier
}
