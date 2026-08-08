#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT       1
#define LIGHT_TYPE_SPOT        2
#define LIGHT_TYPE_AREA        3

layout (std140, binding = 0) uniform Camera
{
    mat4 projection;
    mat4 view;
    vec4 position; // xyz = camera position, w = free
} camera_data;

uniform mat4 model;

layout (std140, binding = 1) uniform LightData
{
    vec4 position;  // .rgb = position,  .a = cut_off       (cosine of inner cone angle)
    vec4 direction; // .rgb = direction, .a = outer_cut_off (cosine of outer cone angle)
    vec4 ambient;   // .rgb = ambient,   .a = radius        (attenuation radius)
    vec4 diffuse;   // .rgb = diffuse,   .a = intensity
    vec4 specular;  // .rgb = specular,  .a = PADDING
    int type;
    float padding[3];
} light_data;

uint pcg_hash(uint input_value)
{
    uint state = input_value * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

float pcg_hash_rand01(uint seed)
{
    return float(pcg_hash(seed)) / 4294967295.0; // normalize to 0.0-1.0
}

vec3 apply_billboarding(
    vec3 world_position,
    vec2 vertex_xy,
    float rotation,
    float scale,
    mat4 view)
{
    vec3 camera_right = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 camera_up = vec3(view[0][1], view[1][1], view[2][1]);

    vec2 centered_vertex = vertex_xy - vec2(0.5, 0.5);

    float rotation_radians = radians(rotation);
    float cos_theta = cos(rotation_radians);
    float sin_theta = sin(rotation_radians);

    vec2 rotated_vertex = vec2(
    cos_theta * centered_vertex.x - sin_theta * centered_vertex.y,
    sin_theta * centered_vertex.x + cos_theta * centered_vertex.y
    );

    return world_position + (camera_right * rotated_vertex.x + camera_up * rotated_vertex.y) * scale;
}

float get_light_distance(vec3 frag_position)
{
    return length(light_data.position.xyz - frag_position);
}

vec3 get_light_direction(vec3 frag_position)
{
    return normalize(light_data.position.xyz - frag_position);
}

vec3 get_view_direction(vec3 frag_position)
{
    return normalize(camera_data.position.xyz - frag_position);
}

// Smooth radial alpha falloff from opaque center to transparent edges (like default_particle.png)
// Higher softness = softer edges, lower softness = closer to a hard circle
float calculate_point_alpha_falloff(vec2 point_coord, float softness)
{
    vec2 circle_coord = point_coord * 2.0 - 1.0;
    float radius = length(circle_coord);
    return 1.0 - smoothstep(1.0 - softness, 1.0, radius);
}

float process_attenuation(vec3 frag_position)
{
    float distance = get_light_distance(frag_position);
    float radius = light_data.ambient.a;
    float c = 1.0;
    float l = 4.5 / radius;
    float q = 75.0 / (radius * radius);
    return 1.0 / (c + l * distance + q * (distance * distance));
}

// Forward scatter: cheap translucency effect where light passes through semi-transparent particle and lights it from behind.
// Normal not applied here since it is not a surface-based position.
float calculate_translucency(vec3 view_direction, vec3 light_direction, float scatter_power)
{
    float back_scatter = max(dot(-view_direction, light_direction), 0.0);
    return pow(back_scatter, scatter_power);
}

vec3 process_particle_light(vec3 frag_position, vec3 view_direction, float scatter_power)
{
    vec3 light_direction;
    float attenuation = 1.0;
    float spot_intensity = 1.0;

    if (light_data.type == LIGHT_TYPE_DIRECTIONAL)
    {
        light_direction = normalize(-light_data.direction.xyz);
    }
    else
    {
        light_direction = get_light_direction(frag_position);
        attenuation = process_attenuation(frag_position);

        if (light_data.type == LIGHT_TYPE_SPOT || light_data.type == LIGHT_TYPE_AREA)
        {
            vec3 world_spotlight_direction = normalize(light_data.direction.xyz);
            float theta = dot(light_direction, -world_spotlight_direction);
            float cut_off = light_data.position.a;
            float outer_cut_off  = light_data.direction.a;
            float epsilon = cut_off - outer_cut_off;
            spot_intensity = clamp((theta - outer_cut_off) / max(epsilon, 0.001), 0.0, 1.0);
        }
    }

    float translucency = calculate_translucency(view_direction, light_direction, scatter_power);

    vec3 ambient = light_data.ambient.rgb;
    vec3 scattered = light_data.diffuse.rgb * translucency;

    return (ambient + scattered) * attenuation * spot_intensity * light_data.diffuse.a;
}