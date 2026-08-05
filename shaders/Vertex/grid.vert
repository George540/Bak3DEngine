#version 460 core

layout (location = 0) in vec3 aPos;

#include "Common_Global.glsl"

void main()
{
	mat4 modelViewProjection = camera_data.projection * camera_data.view * model;
	gl_Position = modelViewProjection * vec4(aPos, 1.0);
}