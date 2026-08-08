#version 460

out vec4 fragColor;

in VS_OUT
{
    vec3 frag_position;
} fs_in;

#include "Common_Global.glsl"

void main()
{
    vec3 frag_normal;
    if (!get_point_sphere_normal(gl_PointCoord, fs_in.frag_position, camera_data.position.xyz, frag_normal))
    {
        discard;
    }

    vec3 lit_result = vec3(0.0);

    switch (light_data.type)
    {
        case LIGHT_TYPE_DIRECTIONAL: lit_result = process_directional_light(fs_in.frag_position, frag_normal); break;
        case LIGHT_TYPE_POINT:       lit_result = process_point_light(fs_in.frag_position, frag_normal); break;
        case LIGHT_TYPE_SPOT:        lit_result = process_spot_light(fs_in.frag_position, frag_normal); break;
        case LIGHT_TYPE_AREA:        lit_result = process_spot_light(fs_in.frag_position, frag_normal); break;
        default:                     lit_result = vec3(1.0, 1.0, 1.0); break;
    }

    fragColor = vec4(lit_result, 1.0);
}
