#version 460 core

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

struct Particle
{
    vec4 position; // XYZ = world-space position, W = world-space particle size (radius units)
};

//@TODO: Add to particle config UBO
uniform int particle_count;
uniform float spawn_extent = 5.0; // half-extent of the world-space scatter cube, in world units

layout(std430, binding = 16) buffer ParticleBuffer
{
    Particle particles[];
};

#include "Common_Global.glsl"

void main()
{
    uint index = gl_GlobalInvocationID.x;
    if (index >= particle_count) // NUM_PARTICLES needs to be passed in via a uniform or UBO
    {
        return;
    }

    Particle particles_sorted[particle_count];
}
