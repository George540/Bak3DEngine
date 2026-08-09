#version 460

out vec4 fragColor;

in VS_OUT
{
    vec3 frag_position;
} fs_in;

//@TODO: Add to particle config UBO
uniform vec3 particle_albedo = vec3(1.0);
uniform float scatter_power = 10.0;
uniform float edge_softness = 0.7;

#include "Common_Global.glsl"

void main()
{
    vec3 frag_normal;
    float alpha = calculate_point_alpha_falloff(gl_PointCoord, edge_softness);
    if (alpha <= 0.01)
    {
        discard;
    }

    vec3 view_direction = get_view_direction(fs_in.frag_position);
    vec3 lighting = process_particle_light(fs_in.frag_position, view_direction, scatter_power);

    fragColor = vec4(particle_albedo * lighting, alpha);
}
