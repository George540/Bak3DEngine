#version 460 core

in vec2 TexCoords;

uniform vec4 diffuseColor = vec4(1.0);
uniform sampler2D sprite;

out vec4 FragColor;

void main()
{
	FragColor = (texture(sprite, TexCoords) * diffuseColor);
}