/* ===========================================================================
The MIT License (MIT)

Copyright (c) 2022-2026 George Mavroeidis - GeoGraphics

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
=========================================================================== */

#pragma once
#include "Buffers/data_buffer.h"
#include "Buffers/frame_buffer.h"
#include "Scene/Objects/light.h"

struct ShadowGPUData
{
    glm::mat4 light_space_matrix = glm::mat4(1.0f);
    glm::vec4 bias_params = glm::vec4(0.0f);       // x = const bias, y = slope bias, z = normal offset (all shadow texels), w = PCF radius (texels)
    glm::vec4 projection_params = glm::vec4(0.0f); // x = near, y = far, z = frustum scale (ortho half-extent | tan(half fov)), w = 1 perspective / 0 ortho
    glm::vec4 map_params = glm::vec4(0.0f);        // x = resolution, y = enabled
};
static constexpr GLsizei SHADOW_GPU_DATA_SIZE = sizeof(ShadowGPUData);
static_assert(SHADOW_GPU_DATA_SIZE == 112, "Must match ShadowEntry std430 layout");

// Tunable settings collected in a single payload
struct ShadowSettings
{
    GLuint resolution = 2048; // read once at initialize()

    float depth_bias_texels = 1.0f;
    float slope_bias_texels = 2.0f;
    float normal_offset_texels = 1.5f;
    int pcf_radius = 1; // 1 = 3x3, 2 = 5x5
    bool cull_front_faces = true; // render back faces into the map (acne + peter panning); single-sided planes won't cast

    float near_plane = 0.1f;

    GLuint initial_layers = 4; // Starting array capacity, which grows by doubling to keep reallocation rare
    GLuint max_casters = 0; // 0 = limited only by GL_MAX_ARRAY_TEXTURE_LAYERS (which is 2048)

    // Directional (orthographic box)
    glm::vec3 directional_focus = glm::vec3(0.0f);
    float directional_extent = 25.0f; // half-size of the box in world units
    float directional_depth_range = 100.0f;

    // Spot (perspective)
    float spot_max_range = 100.0f;
    float spot_fov_margin_degrees = 2.0f;
};

/*
 * Renders a single shadow map (first active light) and uploads matrices/params to the GPU.
 */
class ShadowRenderer
{
public:
    static constexpr GLuint SHADOW_DATA_SSBO_BINDING = 14;
    static constexpr GLuint SHADOW_MAP_TEXTURE_UNIT = 8; // must match layout(binding = 8) in Common_Global.glsl

    static void initialize();
    static void shutdown();
    static void render();

    static int get_shadow_index(const Light* light);
    static ShadowSettings& get_settings() { return m_settings; }

private:
    static ShadowGPUData build_shadow_data(const Light& light);
    static void ensure_layer_capacity(size_t needed);
    static void update_and_upload_data();
    static void render_depth_pass(const ShaderRef& shader);

    static std::unique_ptr<ShadowMapFrameBuffer> m_shadow_fbo;
    static std::unique_ptr<ShaderStorageBuffer> m_shadow_ssbo;
    static std::vector<const Light*> m_light_casters;
    static std::vector<ShadowGPUData> m_shadow_data;
    static GLuint m_max_layers;
    static ShadowSettings m_settings;
};
