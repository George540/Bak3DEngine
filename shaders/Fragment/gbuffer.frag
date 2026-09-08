#version 460 core

struct Material
{
    bool use_diffuse_texture;
    bool use_specular_texture;
    bool use_normal_texture;
    bool use_gamma_correction;
    float gamma;
    vec4 surface_parameters; // x=ambient, y=diffuse, z=specular, w=shininess

    sampler2D diffuse_texture;
    sampler2D specular_texture;
    sampler2D normal_texture;
};

in VS_OUT
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
    vec3 Tangent;
    vec3 Bitangent;
} fs_in;

uniform Material material;

layout(location = 0) out vec4 g_position;
layout(location = 1) out vec4 g_normal;
layout(location = 2) out vec4 g_albedo_specular;
layout(location = 3) out vec4 g_material;

vec3 get_world_normal()
{
    // TBM setup
    vec3 n = normalize(fs_in.Normal);
    if (!material.use_normal_texture)
    {
        return n;
    }

    vec3 t = normalize(fs_in.Tangent);
    vec3 b = normalize(fs_in.Bitangent);
    mat3 TBN = mat3(t, b, n);

    vec3 sampled = texture(material.normal_texture, fs_in.TexCoord).rgb * 2.0 - 1.0;
    return normalize(TBN * sampled);
}

void main()
{
    g_position = vec4(fs_in.FragPos, 1.0);
    g_normal = vec4(get_world_normal(), 1.0); // .a = 1.0 marks this pixel as GBuffer-written

    vec3 albedo = material.use_diffuse_texture
                ? texture(material.diffuse_texture, fs_in.TexCoord).rgb
                : vec3(material.surface_parameters.y);

    float specular = material.use_specular_texture
                ? texture(material.specular_texture, fs_in.TexCoord).r
                : material.surface_parameters.z;

    g_albedo_specular = vec4(albedo, specular);

    vec3 ambient_basis = material.use_diffuse_texture
                ? albedo
                : vec3(material.surface_parameters.x);

    g_material = vec4(ambient_basis, material.surface_parameters.w / 256.0);
}
