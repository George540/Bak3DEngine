#version 460 core

layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

uniform float icon_size_multiplier = 1.0;

out vec2 TexCoords;

#include "Common_Global.glsl"

void main()
{
    // Extract world position from the model matrix's translation column
    vec3 worldPos = vec3(model[3]);

    vec3 worldPosition = apply_billboarding(
        worldPos,
        vertex.xy,
        0.0, // no rotation for a light marker
        0.0,
        icon_size_multiplier,
        camera_data.view
    );

    TexCoords = vertex.zw;
    gl_Position = camera_data.projection * camera_data.view * vec4(worldPosition, 1.0);
}