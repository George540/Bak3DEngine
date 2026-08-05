#version 460 core

in vec2 TexCoord;
in vec3 FragPos;

uniform sampler2D textureSampler;
out vec4 FragColor;

void main()
{
	vec4 textureColor = texture(textureSampler, TexCoord);
	vec3 result = vec3(textureColor.r, textureColor.g, textureColor.b);
	FragColor = vec4(result, 1.0);
}