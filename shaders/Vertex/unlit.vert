#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec2 TexCoord;

#include "Common_Global.glsl"

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    TexCoord = aTexCoords;
    gl_Position = camera_data.projection * camera_data.view * model * vec4(aPos, 1.0);
}