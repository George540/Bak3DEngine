#version 460 core

layout (location = 0) in vec3 aPos;

#include "Common_Global.glsl"

void main()
{
    gl_Position = camera_data.projection * camera_data.view * model * vec4(aPos, 1.0);
}