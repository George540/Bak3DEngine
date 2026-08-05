#version 460 core

struct PostProcess_ColorGrading
{
    bool invert;
    bool grayscale;
    float brightness;
    float contrast;
    float hue;
    float saturation;
    float temperature;
    float vignette_intensity;
    vec4 vignette_color;
};

in vec2 TextCoords;

out vec4 FragColor;

uniform sampler2D screenTexture;
uniform PostProcess_ColorGrading post_process_color_grading;

// Source of conversion functions, made by 983 on Github Gist
// frag.glsl https://gist.github.com/983/e170a24ae8eba2cd174f
vec3 rgb_to_hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0/3.0, 2.0/3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv_to_rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0/3.0, 1.0/3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
    vec4 new_color = vec4(texture(screenTexture, TextCoords).rgb, 1.0);
    
    // Toggled effects
    if (post_process_color_grading.invert)
    {
        new_color = vec4(vec3(1.0 - new_color.rgb), 1.0);
    }
    if (post_process_color_grading.grayscale)
    {
        float luminance = dot(new_color.rgb, vec3(0.2126, 0.7152, 0.0722));
        new_color = vec4(vec3(luminance), 1.0);
    }

    // Continuous effects
    new_color.rgb += post_process_color_grading.brightness;
    new_color.rgb = (new_color.rgb - 0.5) * (post_process_color_grading.contrast + 1.0) + 0.5;

    vec3 hsv = rgb_to_hsv(new_color.rgb);
    hsv.x = fract(hsv.x + post_process_color_grading.hue);
    hsv.y *= post_process_color_grading.saturation + 1.0;
    new_color.rgb = hsv_to_rgb(hsv);
    
    new_color.r += post_process_color_grading.temperature;
    new_color.b -= post_process_color_grading.temperature;

    vec2 uv = TextCoords - 0.5;
    float dist = length(uv);
    float vignette = 1.0 - dist * dist;

    vec3 v_color = post_process_color_grading.vignette_intensity >= 0.0
                    ? post_process_color_grading.vignette_color.rgb
                    : 1.0 - post_process_color_grading.vignette_color.rgb;

    vignette = pow(vignette, abs(post_process_color_grading.vignette_intensity));

    new_color.rgb = mix(v_color, new_color.rgb, vignette);

    new_color.rgb = clamp(new_color.rgb, 0.0, 1.0);
    
    FragColor = new_color;
}
