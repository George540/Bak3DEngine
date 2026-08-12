#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT       1
#define LIGHT_TYPE_SPOT        2
#define LIGHT_TYPE_AREA        3

#define DEBUG_VIEW_DEFAULT 0
#define DEBUG_VIEW_DEPTH   1
#define DEBUG_VIEW_AO      2

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

layout (std140, binding = 2) uniform PageData
{
    vec4 depth_settings; // .r = near, .g = far, .gb = currently unused
    int debug_mode;      // 0 = color, 1 = depth, TBD: 2 = AO, 3 = normals, etc
    float padding[3];
} page_data;

// Nathan Reed, "Hash Function for GPU Rendering", 2021, https://www.reedbeta.com/blog/hash-functions-for-gpu-rendering/
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

float get_linear_depth(vec3 frag_position)
{
    return distance(frag_position, camera_data.position.xyz);
}

float calculate_wboit_weight(float depth, float alpha)
{
    float weight = alpha * max(1e-2, min(3e3, 0.03 / (1e-5 + pow(depth / 5.0, 4.0))));
    return weight;
}

bool get_point_sphere_normal(vec2 point_coord, mat4 view, out vec3 world_normal)
{
    vec2 circle_coord = point_coord * 2.0 - 1.0;

    float r2 = dot(circle_coord, circle_coord);
    if (r2 > 1.0)
    {
        world_normal = vec3(0.0);
        return false;
    }

    // Sphere equation based on z:
    // x^2 + y^2 + z^2 = 1 -> z = sqrt(1.0 - x^2 - y^2)
    float z = sqrt(1.0 - r2);

    // Camera basis vectors in world space (rows of the view matrix)
    vec3 camera_right = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 camera_up = vec3(view[0][1], view[1][1], view[2][1]);
    vec3 camera_forward = vec3(view[0][2], view[1][2], view[2][2]); // points away from camera into scene

    // Flip forward so normal points toward the camera
    world_normal = normalize(circle_coord.x * camera_right + circle_coord.y * camera_up - z * camera_forward);
    return true;
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

// Enhanced translucency handling peak alignment and side profile lighting
float calculate_translucency(vec3 view_direction, vec3 light_direction, float scatter_power)
{
    float alignment = dot(view_direction, light_direction);

    // Narrow peaks: Handles back-lit and front-lit edge highlights
    float back_scatter = max(-alignment, 0.0);
    float front_scatter = max(alignment, 0.0);
    float directional_scatter = pow(back_scatter + front_scatter, scatter_power);

    // Isotropic base: Simulates internally bounced light that escapes through the sides
    // When alignment is 0.0 (exact side view), this provides a solid 0.5 baseline contribution
    float isotropic_scatter = alignment * 0.5 + 0.5;

    // Linearly blend the two phases
    // 0.2 gives a subtle volumetric density look from the side without flattening your highlights
    return mix(isotropic_scatter, directional_scatter, 0.8);
}

vec3 process_particle_light(vec3 frag_position, vec3 view_direction, float scatter_power)
{
    vec3 light_direction;
    float attenuation = 1.0;
    float light_intensity = 1.0;

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
            light_intensity = clamp((theta - outer_cut_off) / max(epsilon, 0.001), 0.0, 1.0);
        }
    }

    // Directional translucent scattering
    float translucency = calculate_translucency(view_direction, light_direction, scatter_power);

    // Add an isotropic/wrapped diffuse component for side angles (90 degrees)
    // This simulates multi-scattering where light scatters evenly out the sides of the volume.
    float side_scattering = clamp(dot(light_direction, view_direction) * 0.5 + 0.5, 0.0, 1.0);

    // Smoothly blend directional translucency with all-around side scattering
    // Using 0.15 to 0.3 as a baseline ensures the particle doesn't go pitch black from the side.
    float final_scattering = max(translucency, side_scattering * 0.25); // @TODO: To UBO

    vec3 ambient = light_data.ambient.rgb;
    vec3 scattered = light_data.diffuse.rgb * final_scattering;

    return (ambient + scattered) * attenuation * light_intensity * light_data.diffuse.a;
}
