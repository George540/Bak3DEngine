#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aCol;

#include "Common_Global.glsl"

out vec3 vertexColor;

void main()
{
	vertexColor = aCol;
	mat4 modelViewProjection = camera_data.projection * camera_data.view * model;
	gl_Position = modelViewProjection * vec4(aPos, 1.0);
}