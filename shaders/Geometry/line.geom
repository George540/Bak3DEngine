#version 460 core

layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;

uniform vec2 viewport_size = vec2(1920.0, 1080.0);
uniform float line_width = 3.0;

void main()
{
    vec4 p0 = gl_in[0].gl_Position;
    vec4 p1 = gl_in[1].gl_Position;

    // Clip the segment against the near plane in clip space so w stays positive
    float d0 = p0.z + p0.w;
    float d1 = p1.z + p1.w;
    if (d0 < 0.0 && d1 < 0.0)
    {
        return;
    }
    if (d0 < 0.0)
    {
        p0 = mix(p0, p1, d0 / (d0 - d1));
    }
    else if (d1 < 0.0)
    {
        p1 = mix(p1, p0, d1 / (d1 - d0));
    }

    // Perpendicular computed in pixel space so the width is uniform on screen
    vec2 pixel_dir = (p1.xy / p1.w - p0.xy / p0.w) * viewport_size;
    float pixel_len = length(pixel_dir);
    pixel_dir = pixel_len > 1e-6 ? pixel_dir / pixel_len : vec2(1.0, 0.0);

    // Half-width in pixels
    vec2 ndc_offset = vec2(-pixel_dir.y, pixel_dir.x) * line_width / viewport_size;

    // Multiply by w to go back to clip space
    gl_Position = vec4(p0.xy + ndc_offset * p0.w, p0.z, p0.w); EmitVertex();
    gl_Position = vec4(p0.xy - ndc_offset * p0.w, p0.z, p0.w); EmitVertex();
    gl_Position = vec4(p1.xy + ndc_offset * p1.w, p1.z, p1.w); EmitVertex();
    gl_Position = vec4(p1.xy - ndc_offset * p1.w, p1.z, p1.w); EmitVertex();
    EndPrimitive();
}