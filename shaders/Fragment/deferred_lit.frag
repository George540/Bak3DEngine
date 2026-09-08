#version 460 core

in vec2 TextCoords;
out vec4 FragColor;

uniform sampler2D g_position;
uniform sampler2D g_normal;
uniform sampler2D g_albedo_specular;
uniform sampler2D g_material;
uniform int light_count;

#include "Common_Global.glsl"

void main()
{
    vec4 normal_sample = texture(g_normal, TextCoords);
    if (normal_sample.a < 0.5)
    {
        discard; // pixel was never written by the GBuffer pass (background / forward territory)
    }

    vec3 frag_position = texture(g_position, TextCoords).rgb;
    vec3 normal = normal_sample.rgb;
    vec4 albedo_specular = texture(g_albedo_specular, TextCoords);
    vec4 mat_sample = texture(g_material, TextCoords);

    vec3 albedo = albedo_specular.rgb;
    float specular_strength = albedo_specular.a;
    vec3 ambient_basis = mat_sample.rgb;
    float shininess = mat_sample.a * 256.0;

    vec3 view_direction = normalize(camera_data.position.xyz - frag_position);
    vec3 result = vec3(0.0);

    for (int i = 0; i < light_count; ++i)
    {
        result += process_deferred_light_contribution(light_buffer.lights[i], frag_position, normal, albedo, ambient_basis, specular_strength, shininess, view_direction);
    }

    FragColor = vec4(result, 1.0);
}
