#version 460 core

in vec3 vertexColor;
out vec4 FragColor;

void main()
{
	FragColor = vec4(vertexColor.r, vertexColor.g, vertexColor.b, 1.0f);
}