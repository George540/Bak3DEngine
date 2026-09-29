#version 460 core

layout (location = 0) in vec3 aPos;

#include "Common_Global.glsl"

void main()
{
    gl_Position = shadow_data.light_space_matrix * model * vec4(aPos, 1.0);
}
