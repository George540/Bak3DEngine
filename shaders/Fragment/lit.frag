#version 460 core

struct Material
{
    bool use_diffuse_texture;
    bool use_specular_texture;
    bool use_normal_texture;
    bool use_gamma_correction;
    float gamma;
    
    vec4 surface_parameters; // x = ambient, y = diffuse z = specular, w = shininess

    sampler2D diffuse_texture;
    sampler2D specular_texture;
    sampler2D normal_texture;
};

in VS_OUT
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
    mat3 TBN;
} fs_in;

uniform Material material;

out vec4 FragColor;

#include "Common_Global.glsl"

// Helpers

vec3 get_normal()
{
    if (material.use_normal_texture)
    {
        vec3 normal_map_value = texture(material.normal_texture, fs_in.TexCoord).rgb;
        normal_map_value = normalize(normal_map_value * 2.0 - 1.0);  // tangent space [-1, 1]
        return normalize(fs_in.TBN * normal_map_value);
    }
    return normalize(fs_in.Normal);
}

vec3 get_view_dir()
{
    return normalize(camera_data.position.xyz - fs_in.FragPos);
}

vec3 calc_ambient(vec3 light_ambient)
{
    if (material.use_diffuse_texture)
        return light_ambient * texture(material.diffuse_texture, fs_in.TexCoord).rgb;
    return light_ambient * vec3(material.surface_parameters.x);
}

vec3 calc_diffuse(vec3 light_diffuse, vec3 light_direction, vec3 normal)
{
    float diff = max(dot(light_direction, normal), 0.0);
    if (material.use_diffuse_texture)
        return light_diffuse * diff * texture(material.diffuse_texture, fs_in.TexCoord).rgb;
    return light_diffuse * diff * vec3(material.surface_parameters.y);
}

vec3 calc_specular(vec3 light_specular, vec3 light_direction, vec3 normal, vec3 viewDir)
{
    vec3 halfway_direction = normalize(light_direction + viewDir);
    float spec = pow(max(dot(normal, halfway_direction), 0.0), material.surface_parameters.w);
    if (material.use_specular_texture)
        return light_specular * spec * texture(material.specular_texture, fs_in.TexCoord).rgb;
    return light_specular * spec * vec3(material.surface_parameters.z);
}

// Light type calculators

vec3 calc_directional_light(LightData light, vec3 normal, vec3 viewDir)
{
    vec3 light_direction = normalize(-light.direction.rgb);

    vec3 ambient = calc_ambient(light.ambient.rgb);
    vec3 diffuse = calc_diffuse(light.diffuse.rgb, light_direction, normal);
    vec3 specular = calc_specular(light.specular.rgb, light_direction, normal, viewDir);

    return (ambient + diffuse + specular) * light.diffuse.a;
}

vec3 calc_point_light(LightData light, vec3 normal, vec3 viewDir)
{
    vec3 light_direction = normalize(light.position.rgb - fs_in.FragPos);

    vec3 ambient = calc_ambient(light.ambient.rgb);
    vec3 diffuse = calc_diffuse(light.diffuse.rgb, light_direction, normal);
    vec3 specular = calc_specular(light.specular.rgb, light_direction, normal, viewDir);

    // Dynamic distance attenuation pass (using light.ambient.a as radius)
    float attenuation = process_attenuation(light, fs_in.FragPos);;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular) * light.diffuse.a;
}

vec3 calc_spot_light(LightData light, vec3 normal, vec3 viewDir)
{
    vec3 light_direction = normalize(light.position.rgb - fs_in.FragPos);

    vec3 ambient  = calc_ambient(light.ambient.rgb);
    vec3 diffuse  = calc_diffuse(light.diffuse.rgb, light_direction, normal);
    vec3 specular = calc_specular(light.specular.rgb, light_direction, normal, viewDir);

    vec3 world_spotlight_direction = normalize(light.direction.rgb);

    // Spotlight cone calculation
    float theta          = dot(light_direction, -world_spotlight_direction);
    float cut_off        = light.position.a;
    float outer_cut_off  = light.direction.a;
    float epsilon        = cut_off - outer_cut_off;
    float spot_intensity = clamp((theta - outer_cut_off) / max(epsilon, 0.001), 0.0, 1.0);

    ambient  *= spot_intensity;
    diffuse  *= spot_intensity;
    specular *= spot_intensity;

    // Distance attenuation pass
    float attenuation = process_attenuation(light, fs_in.FragPos);;

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular) * light.diffuse.a;
}

void main()
{
    vec3 result = vec3(0.0);

    vec3 normal = get_normal();
    vec3 view_direction = get_view_dir();

    for (int i = 0; i < active_light_count; ++i)
    {
        LightData light = light_buffer.lights[i];
        vec3 light_contribution = vec3(0.0);

        switch (light.type)
        {
            case LIGHT_TYPE_DIRECTIONAL: light_contribution = calc_directional_light(light, normal, view_direction); break;
            case LIGHT_TYPE_POINT:       light_contribution = calc_point_light(light, normal, view_direction);       break;
            case LIGHT_TYPE_SPOT:        light_contribution = calc_spot_light(light, normal, view_direction);        break;
            case LIGHT_TYPE_AREA:        light_contribution = calc_spot_light(light, normal, view_direction);        break;
            default:                     light_contribution = vec3(0.0);                                      break;
        }

        result += light_contribution;
    }

    if (material.use_gamma_correction)
    {
        result = pow(result, vec3(1.0 / material.gamma));
    }

    FragColor = vec4(result, 1.0);
}