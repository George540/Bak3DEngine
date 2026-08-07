#version 460

out vec4 fragColor;

#include "Common_Global.glsl"

uniform vec3 particle_albedo = vec3(1.0);

void main()
{
    vec3 normal;
    if (!get_point_sphere_normal(gl_PointCoord, camera_data.view, normal))
    {
        discard;
    }

    //vec3 view_dir = normalize(camera_data_position_placeholder - FragPos);

    vec3 lit_result = particle_albedo; // @TODO: Include light data switch setup

    fragColor = vec4(lit_result, 1.0);
}
