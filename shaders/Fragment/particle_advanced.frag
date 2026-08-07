#version 460

out vec4 fragColor;

void main()
{
    vec2 circle_coord = 2.0 * gl_PointCoord - 1.0;
    
    // To shape the point particles into a circle,
    // throw away squared corners if fragment lies outside the circle's unit radius
    if (dot(circle_coord, circle_coord) > 1.0)
    {
        discard;
    }
    fragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
