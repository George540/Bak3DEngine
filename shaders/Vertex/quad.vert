#version 460 core

layout (location = 0) in vec4 position;

out vec2 TextCoords;

void main()
{
    // Convert 0..1 range to -1..1
    gl_Position = vec4(position.xy * 2.0 - 1.0, 0.0, 1.0);
    TextCoords = position.zw;
}
