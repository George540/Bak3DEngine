#version 460 core

// Shared quad vertex (pos.xy, uv.xy)
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

// Per-instance data (divisor = 1)
layout (location = 1) in vec4 instancePosition; // <vec3 position, float rotation>
layout (location = 2) in vec4 instanceColor;
layout (location = 3) in float instanceScale;

out vec2 TexCoords;
out vec4 ParticleColor;

#include "Common_Global.glsl"

void main()
{
    // Discard dead particles on GPU side (alpha == 0 after clip)
    // We still emit the vertex; the fragment shader discards via alpha.
    // Alternatively clip here for early-out:
    if (instanceColor.a <= 0.0)
    {
        // Push outside clip space, effectively culls the instance
        gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
        ParticleColor = vec4(0.0);
        TexCoords = vec2(0.0);
        return;
    }

    // Apply billboarding to instance quad
    vec3 worldPosition = apply_billboarding(
        instancePosition.xyz,
        vertex.xy,
        instancePosition.w,
        instanceScale,
        camera_data.view
    );

    // Set texture coordinates and particle color
    TexCoords = vertex.zw;
    ParticleColor = instanceColor;

    // Apply the projection and view matrices
    gl_Position = camera_data.projection * camera_data.view * vec4(worldPosition, 1.0);
}