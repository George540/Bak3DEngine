#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

#include "Common_Global.glsl"

out VS_OUT
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
    vec3 Tangent;
    vec3 Bitangent;
} vs_out;

void main()
{
    vs_out.FragPos = vec3(model * vec4(position, 1.0));
    vs_out.TexCoord = tex_coord;
    vs_out.Normal = mat3(transpose(inverse(model))) * normal;
    vs_out.Tangent = normalize(vec3(model * vec4(tangent, 0.0)));
    vs_out.Bitangent = normalize(vec3(model * vec4(bitangent, 0.0)));

    gl_Position = camera_data.projection * camera_data.view * model * vec4(position, 1.0);
}
