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

#include "shadow_renderer.h"

#include "debug_scope.h"
#include "Asset/resource_manager.h"
#include "Scene/scene_manager.h"

using namespace std;

unique_ptr<ShadowMapFrameBuffer> ShadowRenderer::m_shadow_fbo;
unique_ptr<ShaderStorageBuffer> ShadowRenderer::m_shadow_ssbo;
vector<const Light*> ShadowRenderer::m_light_casters;
vector<ShadowGPUData> ShadowRenderer::m_shadow_data;
GLuint ShadowRenderer::m_max_layers = 1;
ShadowSettings ShadowRenderer::m_settings;

void ShadowRenderer::initialize()
{
    GLint hardware_layers = 1;
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &hardware_layers);
    m_max_layers = static_cast<GLuint>(hardware_layers);
    if (m_settings.max_casters > 0)
    {
        m_max_layers = glm::min(m_max_layers, m_settings.max_casters);
    }

    const GLuint layers = glm::clamp(m_settings.initial_layers, 1u, m_max_layers);
    m_shadow_fbo = make_unique<ShadowMapFrameBuffer>(m_settings.resolution, layers, "ShadowMap");
    m_shadow_ssbo = make_unique<ShaderStorageBuffer>(0, nullptr, SHADOW_DATA_SSBO_BINDING, GL_DYNAMIC_DRAW);
}

void ShadowRenderer::shutdown()
{
    m_light_casters.clear();
    m_shadow_data.clear();
    m_shadow_fbo.reset();
    m_shadow_ssbo.reset();
}

void ShadowRenderer::render()
{
    update_and_upload_data();

    if (m_light_casters.empty())
    {
        return;
    }

    const ShaderRef shader = ResourceManager::get_shader("shadow_depth");

    glActiveTexture(GL_TEXTURE0 + SHADOW_MAP_TEXTURE_UNIT);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glActiveTexture(GL_TEXTURE0);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glCullFace(m_settings.cull_front_faces ? GL_FRONT : GL_BACK);

    shader->use();
    for (size_t layer = 0; layer < m_light_casters.size(); ++layer)
    {
        DebugScopeGroup scope("Shadow Layer");

        m_shadow_fbo->bind_layer(static_cast<GLuint>(layer));
        glClear(GL_DEPTH_BUFFER_BIT);

        const glm::mat4& light_space = m_shadow_data[layer].light_space_matrix;
        shader->set_mat4("light_space_matrix", light_space);
        const Frustum light_frustum = Frustum::get_frustum_structure(light_space);

        for (const Mesh* mesh : SceneManager::get_current_scene()->get_all_meshes())
        {
            if (!mesh->is_visible() || !mesh->has_mesh())
            {
                continue;
            }
            const auto* data = dynamic_cast<const MeshData*>(mesh->get_mesh().operator->());
            if (!data)
            {
                continue;
            }

            // Per-light culling with the mesh's world-space bounding sphere
            const glm::mat4 model = mesh->transform.get_global_model_matrix();
            const glm::vec3 scale = mesh->transform.get_global_scale();
            const glm::vec3 center = glm::vec3(model * glm::vec4(data->get_bounds_center(), 1.0f));
            const float radius = data->get_bounds_radius() * glm::max(scale.x, glm::max(scale.y, scale.z));
            if (!light_frustum.intersects_sphere(center, radius))
            {
                continue;
            }

            shader->set_mat4("model", model);
            data->draw();
        }
    }
    shader->unuse();

    glCullFace(GL_BACK);
    m_shadow_fbo->unbind();

    // Leave the array bound for the forward and deferred passes
    glActiveTexture(GL_TEXTURE0 + SHADOW_MAP_TEXTURE_UNIT);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_shadow_fbo->get_depth_texture());
    glActiveTexture(GL_TEXTURE0);
}

int ShadowRenderer::get_shadow_index(const Light* light)
{
    const auto it = ranges::find(m_light_casters, light);
    return it == m_light_casters.end() ? -1 : static_cast<int>(it - m_light_casters.begin());
}

ShadowGPUData ShadowRenderer::build_shadow_data(const Light& light)
{
    const ShadowSettings& settings = m_settings;
    const glm::vec3 direction = glm::normalize(light.get_direction());
    const glm::vec3 up = glm::abs(direction.y) > 0.99f ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);

    glm::vec3 eye;
    glm::mat4 projection;
    float far_plane;
    float frustum_scale;
    float is_perspective;

    if (light.get_type() == LightType::Directional)
    {
        far_plane = settings.directional_depth_range;
        frustum_scale = settings.directional_extent;
        is_perspective = 0.0f;
        eye = settings.directional_focus - direction * (far_plane * 0.5f); // box centered on focus along the light axis
        projection = glm::ortho(-frustum_scale, frustum_scale, -frustum_scale, frustum_scale, settings.near_plane, far_plane);
    }
    else // Spotlight for now, others will follow
    {
        // Cone half-angle = outer angle + cone size (see Light::recompute_cone_cutoffs)
        const float half_angle = light.get_cone_angle_outer_cutoff() + light.get_cone_size() + settings.spot_fov_margin_degrees;
        const float fov = glm::radians(glm::min(half_angle * 2.0f, 170.0f));
        far_plane = glm::min(light.get_effective_radius(), settings.spot_max_range);
        frustum_scale = glm::tan(fov * 0.5f);
        is_perspective = 1.0f;
        eye = light.transform.get_global_position();
        projection = glm::perspective(fov, 1.0f, settings.near_plane, far_plane);
    }

    ShadowGPUData data;
    data.light_space_matrix = projection * glm::lookAt(eye, eye + direction, up);
    data.bias_params = glm::vec4(settings.depth_bias_texels, settings.slope_bias_texels, settings.normal_offset_texels, static_cast<float>(settings.pcf_radius));
    data.projection_params = glm::vec4(settings.near_plane, far_plane, frustum_scale, is_perspective);
    data.map_params = glm::vec4(static_cast<float>(settings.resolution), 1.0f, 0.0f, 0.0f);
    return data;
}

void ShadowRenderer::ensure_layer_capacity(size_t needed)
{
    const GLuint current = m_shadow_fbo->get_layer_count();
    if (needed <= current)
    {
        return;
    }
    const GLuint next = glm::min(glm::max(static_cast<GLuint>(needed), current * 2), m_max_layers);
    m_shadow_fbo->resize_layers(next);
}

void ShadowRenderer::update_and_upload_data()
{
    if (m_shadow_data.empty())
    {
        return;
    }

    const auto size = static_cast<GLsizeiptr>(m_shadow_data.size()) * SHADOW_GPU_DATA_SIZE;
    m_shadow_ssbo->bind();
    m_shadow_ssbo->bind_buffer_data(m_shadow_data.data(), size);
    m_shadow_ssbo->bind_to_binding_point(SHADOW_DATA_SSBO_BINDING);
}

void ShadowRenderer::render_depth_pass(const ShaderRef& shader)
{
    // Avoid a feedback loop: the map must not be bound for sampling while it is the render target
    glActiveTexture(GL_TEXTURE0 + SHADOW_MAP_TEXTURE_UNIT);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);

    m_shadow_fbo->bind(); // also sets the viewport to the map resolution
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glCullFace(m_settings.cull_front_faces ? GL_FRONT : GL_BACK);

    shader->use();
    for (const Mesh* mesh : SceneManager::get_current_scene()->get_all_meshes())
    {
        if (!mesh->is_visible() || !mesh->has_mesh())
        {
            continue;
        }
        shader->set_mat4("model", mesh->transform.get_global_model_matrix());
        mesh->get_mesh()->draw();
    }
    shader->unuse();

    glCullFace(GL_BACK);
    m_shadow_fbo->unbind();

    // Leave the map bound for the geometry passes
    glActiveTexture(GL_TEXTURE0 + SHADOW_MAP_TEXTURE_UNIT);
    glBindTexture(GL_TEXTURE_2D, m_shadow_fbo->get_depth_texture());
    glActiveTexture(GL_TEXTURE0);
}
