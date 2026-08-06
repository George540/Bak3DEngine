#version 460

// Layout 0 points to our shared buffer array
layout(location = 0) in vec4 inParticle;

void main()
{
    gl_Position = vec4(inParticle.xyz, 1.0f);
    gl_PointSize = inParticle.w;
}
