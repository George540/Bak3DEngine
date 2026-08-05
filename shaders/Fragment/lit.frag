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
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    vec3 TangentLightDir;
} fs_in;

uniform Material material;
uniform vec3 camera_position;

out vec4 FragColor;

#include "Common_Global.glsl"

// Helpers

vec3 get_normal()
{
    if (material.use_normal_texture)
    {
        vec3 n = texture(material.normal_texture, fs_in.TexCoord).rgb;
        return normalize(n * 2.0 - 1.0); // tangent space [-1, 1]
    }
    return normalize(fs_in.Normal);
}

vec3 get_light_dir(vec3 light_pos)
{
    if (material.use_normal_texture)
        return normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    return normalize(light_pos - fs_in.FragPos);
}

vec3 get_view_dir()
{
    if (material.use_normal_texture)
        return normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    return normalize(camera_position - fs_in.FragPos);
}

vec3 calc_ambient(vec3 light_ambient)
{
    if (material.use_diffuse_texture)
        return light_ambient * texture(material.diffuse_texture, fs_in.TexCoord).rgb;
    return light_ambient * vec3(material.surface_parameters.x);
}

vec3 calc_diffuse(vec3 light_diffuse, vec3 lightDir, vec3 normal)
{
    float diff = max(dot(lightDir, normal), 0.0);
    if (material.use_diffuse_texture)
        return light_diffuse * diff * texture(material.diffuse_texture, fs_in.TexCoord).rgb;
    return light_diffuse * diff * vec3(material.surface_parameters.y);
}

vec3 calc_specular(vec3 light_specular, vec3 lightDir, vec3 normal, vec3 viewDir)
{
    vec3  halfwayDir = normalize(lightDir + viewDir);
    float spec       = pow(max(dot(normal, halfwayDir), 0.0), material.surface_parameters.w);
    if (material.use_specular_texture)
        return light_specular * spec * texture(material.specular_texture, fs_in.TexCoord).rgb;
    return light_specular * spec * vec3(material.surface_parameters.z);
}

float calc_attenuation(float dist)
{
    float radius = light_data.ambient.a;
    float c = 1.0;
    float l = 4.5  / radius;
    float q = 75.0 / (radius * radius);
    return 1.0 / (c + l * dist + q * (dist * dist));
}

// Light type calculators

vec3 calc_directional_light()
{
    vec3 lightDir = material.use_normal_texture
                    ? normalize(fs_in.TangentLightDir)
                    : normalize(-light_data.direction.rgb);
    vec3 normal   = get_normal();
    vec3 viewDir  = get_view_dir();

    vec3 ambient  = calc_ambient(light_data.ambient.rgb);
    vec3 diffuse  = calc_diffuse(light_data.diffuse.rgb, lightDir, normal);
    vec3 specular = calc_specular(light_data.specular.rgb, lightDir, normal, viewDir);

    // No attenuation — directional lights are infinitely distant
    return (ambient + diffuse + specular) * light_data.diffuse.a;
}

vec3 calc_point_light()
{
    vec3 lightDir = get_light_dir(light_data.position.rgb);
    vec3 normal   = get_normal();
    vec3 viewDir  = get_view_dir();
    
    vec3 ambient  = calc_ambient(light_data.ambient.rgb);
    vec3 diffuse  = calc_diffuse(light_data.diffuse.rgb, lightDir, normal);
    vec3 specular = calc_specular(light_data.specular.rgb, lightDir, normal, viewDir);

    float dist        = length(light_data.position.rgb - fs_in.FragPos);
    float attenuation = calc_attenuation(dist);

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular) * light_data.diffuse.a;
}

vec3 calc_spot_light()
{
    vec3 lightDir = get_light_dir(light_data.position.rgb);
    vec3 normal   = get_normal();
    vec3 viewDir  = get_view_dir();

    vec3 ambient  = calc_ambient(light_data.ambient.rgb);
    vec3 diffuse  = calc_diffuse(light_data.diffuse.rgb, lightDir, normal);
    vec3 specular = calc_specular(light_data.specular.rgb, lightDir, normal, viewDir);

    vec3 worldLightDir = normalize(light_data.position.rgb - fs_in.FragPos);
    vec3 worldSpotDir  = normalize(light_data.direction.rgb);

    // Spotlight cone attenuation (soft edges)
    // cut_off and outer_cut_off are stored as cosines, inner > outer
    float theta          = dot(worldLightDir, -worldSpotDir);
    float cut_off        = light_data.position.a;
    float outer_cut_off  = light_data.direction.a;
    float epsilon        = cut_off - outer_cut_off;
    float spot_intensity = clamp((theta - outer_cut_off) / max(epsilon, 0.001), 0.0, 1.0);

    ambient  *= spot_intensity;
    diffuse  *= spot_intensity;
    specular *= spot_intensity;

    // Distance attenuation
    float dist        = length(light_data.position.rgb - fs_in.FragPos);
    float attenuation = calc_attenuation(dist);

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular) * light_data.diffuse.a;
}

void main()
{
    vec3 result = vec3(0.0);

    switch (light_data.type)
    {
        case LIGHT_TYPE_DIRECTIONAL: result = calc_directional_light(); break;
        case LIGHT_TYPE_POINT:       result = calc_point_light();       break;
        case LIGHT_TYPE_SPOT:        result = calc_spot_light();        break;
        case LIGHT_TYPE_AREA:        result = calc_spot_light();        break;
        default:                     result = vec3(1.0, 0.0, 0.0);      break;
    }

    if (material.use_gamma_correction)
    {
        result = pow(result, vec3(1.0 / material.gamma));
    }

    FragColor = vec4(result, 1.0);
}