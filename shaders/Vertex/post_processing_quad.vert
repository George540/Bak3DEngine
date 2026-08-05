#version 460 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TextCoords;

void main()
{
    // Convert 0..1 range to NDC -1..1
    gl_Position = vec4(aPos * 2.0 - 1.0, 0.0, 1.0);
    TextCoords = aTexCoord;
}
