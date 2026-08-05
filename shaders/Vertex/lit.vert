#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

uniform vec3 camera_position;

#include "Common_Global.glsl"

out VS_OUT
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    vec3 TangentLightDir;
} vs_out;

void main()
{
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.TexCoord = aTexCoords;  
    vs_out.Normal = mat3(transpose(inverse(model))) * aNormal;

    vec3 T = normalize(vec3(model * vec4(aTangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(aNormal, 0.0)));
    T = normalize(T - dot(T, N) * N); // re-orthogonalize T with respect to N
    vec3 B = cross(N, T); // then retrieve perpendicular vector B with the cross product of T and N

    // Correct handedness using mesh bitangent
    if (dot(cross(N, T), vec3(model * vec4(aBitangent, 0.0))) < 0.0)
    {
        B = -B;
    }

    mat3 TBN = transpose(mat3(T, B, N));

    vs_out.TangentLightPos = TBN * light_data.position.rgb;
    vs_out.TangentViewPos  = TBN * camera_position;
    vs_out.TangentFragPos  = TBN * vs_out.FragPos;
    vs_out.TangentLightDir = TBN * normalize(-light_data.direction.rgb);

    gl_Position = camera_data.projection * camera_data.view * model * vec4(aPos, 1.0);
}