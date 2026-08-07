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

vec3 process_ambient(vec3 light_ambient)
{
    return light_ambient * vec3(1.0); // 1.0 is surface_parameter.x
}

vec3 process_diffuse(vec3 frag_position, vec3 light_direction, vec3 light_diffuse, vec3 normal)
{
    float diffuse = max(dot(light_direction, normal), 0.0);
    return light_diffuse * diffuse * 1.0; // 1.0 is surface_parameter.y
}

vec3 process_specular(vec3 light_specular, vec3 light_direction, vec3 view_direction, vec3 normal)
{
    vec3 halfway_direction = normalize(light_direction + view_direction);
    float specular = pow(max(dot(normal, halfway_direction), 0.0), 1.0); // 1.0 is surface_parameter.z
    return light_specular * specular * vec3(1.0); // 1.0 is surface_parameter.z
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

vec3 process_directional_light(vec3 frag_position, vec3 normal)
{
    vec3 light_direction = normalize(-light_data.direction.xyz);
    vec3 view_direction = get_view_direction(frag_position);

    vec3 ambient = process_ambient(light_data.ambient.rgb);
    vec3 diffuse = process_diffuse(frag_position, light_direction, light_data.diffuse.rgb, normal);
    vec3 specular = process_specular(light_data.specular.rgb, light_direction, view_direction, normal);

    return (ambient + diffuse + specular) * light_data.diffuse.a;
}

vec3 process_point_light(vec3 frag_position, vec3 normal)
{
    vec3 light_direction = get_light_direction(frag_position);
    vec3 view_direction = get_view_direction(frag_position);

    vec3 ambient = process_ambient(light_data.ambient.rgb);
    vec3 diffuse = process_diffuse(frag_position, light_direction, light_data.diffuse.rgb, normal);
    vec3 specular = process_specular(light_data.specular.rgb, light_direction, view_direction, normal);

    float attenuation = process_attenuation(frag_position);

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular) * light_data.diffuse.a;
}

vec3 process_spotlight(vec3 frag_position, vec3 normal)
{
    vec3 light_direction = get_light_direction(frag_position);
    vec3 view_direction = get_view_direction(frag_position);

    vec3 ambient = process_ambient(light_data.ambient.rgb);
    vec3 diffuse = process_diffuse(frag_position, light_direction, light_data.diffuse.rgb, normal);
    vec3 specular = process_specular(light_data.specular.rgb, light_direction, view_direction, normal);

    vec3 world_light_direction = normalize(light_data.position.rgb - frag_position);
    vec3 world_spotlight_direction = normalize(light_data.direction.rgb);

    // Spotlight cone attenuation (soft edges)
    // cut_off and outer_cut_off are stored as cosines, inner > outer
    float theta = dot(world_light_direction, -world_spotlight_direction);
    float cut_off = light_data.position.a;
    float outer_cut_off = light_data.direction.a;
    float epsilon = cut_off - outer_cut_off;
    float spot_intensity = clamp((theta - outer_cut_off) / max(epsilon, 0.001), 0.0, 1.0);

    ambient *= spot_intensity;
    diffuse *= spot_intensity;
    specular *= spot_intensity;

    // Distance attenuation
    float attenuation = process_attenuation(frag_position);

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular) * light_data.diffuse.a;
}