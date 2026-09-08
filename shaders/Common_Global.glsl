#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT       1
#define LIGHT_TYPE_SPOT        2
#define LIGHT_TYPE_AREA        3

#define DEBUG_VIEW_LIT              0
#define DEBUG_VIEW_GBUFFER_POSITION 1
#define DEBUG_VIEW_GBUFFER_ALBEDO   2
#define DEBUG_VIEW_GBUFFER_NORMALS  3
#define DEBUG_VIEW_GBUFFER_SPECULAR 4
#define DEBUG_VIEW_DEPTH            5
#define DEBUG_VIEW_SHADOWMASK       6
#define DEBUG_VIEW_AO               7
#define DEBUG_VIEW_MAX              8

layout (std140, binding = 0) uniform Camera
{
    mat4 projection;
    mat4 view;
    vec4 position; // xyz = camera position, w = free
} camera_data;

uniform mat4 model;

struct LightData
{
    vec4 position;  // .rgb = position,  .a = cut_off
    vec4 direction; // .rgb = direction, .a = outer_cut_off
    vec4 ambient;   // .rgb = ambient,   .a = radius
    vec4 diffuse;   // .rgb = diffuse,   .a = intensity
    vec4 specular;  // .rgb = specular,  .a = PADDING
    int type;
    float padding[3];
};

layout (std430, binding = 15) buffer LightDataBuffer
{
    LightData lights[];
} light_buffer;

uniform int active_light_count;

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
    // Extract camera alignment vectors from the view matrix
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

    // If scale is 0.0, calculate a dynamic scale based on distance to camera
    float final_scale = scale;
    if (scale == 0.0)
    {
        // Transform the object's world position into view space to get its depth (0-2)
        // that represents the camera forward vector, while view[3][2] is the translation offset.
        float depth = -(view[0][2] * world_position.x +
                        view[1][2] * world_position.y +
                        view[2][2] * world_position.z +
                        view[3][2]);

        // Use depth as the baseline scale factor.
        final_scale = depth * 0.1; // @TODO: Modify constant screen size from editor slider
    }

    return world_position + (camera_right * rotated_vertex.x + camera_up * rotated_vertex.y) * final_scale;
}


float get_light_distance(LightData light, vec3 frag_position)
{
    return length(light.position.xyz - frag_position);
}

vec3 get_light_direction(LightData light, vec3 frag_position)
{
    return normalize(light.position.xyz - frag_position);
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

float process_attenuation(LightData light, vec3 frag_position)
{
    float distance = get_light_distance(light, frag_position);
    float radius = light.ambient.a;
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

vec3 process_deferred_light_contribution(
        LightData light,
        vec3 frag_pos,
        vec3 normal,
        vec3 albedo,
        vec3 ambient_basis,
        float specular_strength,
        float shininess,
        vec3 view_direction)
{
    vec3 light_direction;
    float attenuation = 1.0;
    float spot_intensity = 1.0;

    if (light.type == LIGHT_TYPE_DIRECTIONAL)
    {
        light_direction = normalize(-light.direction.xyz);
    }
    else
    {
        light_direction = normalize(light.position.xyz - frag_pos);
        float light_distance = length(light.position.xyz - frag_pos);
        float radius = light.ambient.a;
        // Attenuation handling
        float c = 1.0;
        float l = 4.5 / radius;
        float q = 75.0 / (radius * radius);
        attenuation = 1.0 / (c + l * light_distance + q * (light_distance * light_distance));


        if (light.type == LIGHT_TYPE_SPOT || light.type == LIGHT_TYPE_AREA)
        {
            vec3 spot_direction = normalize(light.direction.xyz);
            float theta = dot(light_direction, -spot_direction);
            float cut_off = light.position.a;
            float outer_cut_off = light.direction.a;
            float epsilon = cut_off - outer_cut_off;
            spot_intensity = clamp((theta - outer_cut_off) / max(epsilon, 0.001), 0.0, 1.0);
        }
    }

    float diff = max(dot(normal, light_direction), 0.0);
    vec3 halfway_dir = normalize(light_direction + view_direction);
    float spec = pow(max(dot(normal, halfway_dir), 0.0), max(shininess, 1.0));

    vec3 ambient_term = light.ambient.rgb * ambient_basis;
    vec3 diffuse_term = light.diffuse.rgb * diff * albedo;
    vec3 specular_term = light.specular.rgb * spec * specular_strength;

    // spot cone, then distance attenuation, applied per-term
    ambient_term *= spot_intensity;
    diffuse_term *= spot_intensity;
    specular_term *= spot_intensity;
    ambient_term *= attenuation;
    diffuse_term *= attenuation;
    specular_term *= attenuation;

    return (ambient_term + diffuse_term + specular_term) * light.diffuse.a;
}

vec3 process_particle_light_forward_translucency(LightData light, vec3 frag_position, vec3 view_direction, float scatter_power)
{
    vec3 light_direction;
    float attenuation = 1.0;
    float light_intensity = 1.0;

    if (light.type == LIGHT_TYPE_DIRECTIONAL)
    {
        light_direction = normalize(-light.direction.xyz);
    }
    else
    {
        // Compute direction from the specific light's position
        light_direction = normalize(light.position.xyz - frag_position);

        // Pass the light's explicit attenuation radius (.a channel of ambient)
        attenuation = process_attenuation(light, frag_position);

        if (light.type == LIGHT_TYPE_SPOT || light.type == LIGHT_TYPE_AREA)
        {
            vec3 world_spotlight_direction = normalize(light.direction.xyz);
            float theta = dot(light_direction, -world_spotlight_direction);
            float cut_off = light.position.a;
            float outer_cut_off = light.direction.a;
            float epsilon = cut_off - outer_cut_off;
            light_intensity = clamp((theta - outer_cut_off) / max(epsilon, 0.001), 0.0, 1.0);
        }
    }

    // Directional translucent scattering
    float translucency = calculate_translucency(view_direction, light_direction, scatter_power);

    // Add an isotropic/wrapped diffuse component for side angles (90 degrees)
    float side_scattering = clamp(dot(light_direction, view_direction) * 0.5 + 0.5, 0.0, 1.0);

    // Smoothly blend directional translucency with all-around side scattering
    float final_scattering = max(translucency, side_scattering * 0.25);

    vec3 ambient = light.ambient.rgb;
    vec3 scattered = light.diffuse.rgb * final_scattering;

    // Accumulate all factors alongside the light intensity (.a channel of diffuse)
    return (ambient + scattered) * attenuation * light_intensity * light.diffuse.a;
}

vec3 calculate_total_particle_lighting(vec3 frag_position, vec3 view_direction, float scatter_power)
{
    vec3 total_scattered_light = vec3(0.0);

    // Dynamic iteration over the unsized SSBO array up to the active light count uniform
    for (int i = 0; i < active_light_count; ++i)
    {
        // Extract the current light payload chunk locally
        LightData light = light_buffer.lights[i];

        // Accumulate this specific light's translucent volumetric contribution
        total_scattered_light += process_particle_light_forward_translucency(light, frag_position, view_direction, scatter_power);
    }

    return total_scattered_light;
}
