#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

#include "Common_Global.glsl"

out VS_OUT
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
    mat3 TBN;
} vs_out;

void main()
{
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.TexCoord = aTexCoords;

    vec3 N = normalize(mat3(transpose(inverse(model))) * aNormal);
    vs_out.Normal = N;
    vec3 T = normalize(vec3(model * vec4(aTangent, 0.0)));
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    if (dot(cross(N, T), vec3(model * vec4(aBitangent, 0.0))) < 0.0)
    {
        B = -B;
    }

    vs_out.TBN = mat3(T, B, N);

    gl_Position = camera_data.projection * camera_data.view * vec4(vs_out.FragPos, 1.0);
}