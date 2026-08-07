#version 460

layout(location = 0) in vec4 inParticle; // xyz = world position, w = world-space size

#include "Common_Global.glsl"

uniform float viewport_height;
uniform float point_scale = 1.0; // artist-facing tuning knob

void main()
{
    vec3 worldPosition = inParticle.xyz;
    vec4 viewPosition = camera_data.view * vec4(worldPosition, 1.0);
    gl_Position = camera_data.projection * viewPosition;

    float worldSize = inParticle.w;
    
    float pixelScale = camera_data.projection[1][1] * (viewport_height * 0.5);
    gl_PointSize = clamp(worldSize * pixelScale * point_scale / gl_Position.w, 1.0, 10.0); // Tunable clamps
}
