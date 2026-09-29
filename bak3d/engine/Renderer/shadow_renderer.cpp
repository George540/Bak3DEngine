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

#include "Asset/resource_manager.h"
#include "Scene/scene_manager.h"

using namespace std;

unique_ptr<ShadowMapFrameBuffer> ShadowRenderer::m_shadow_fbo;
unique_ptr<UniformBuffer> ShadowRenderer::m_shadow_ubo;
const Light* ShadowRenderer::m_shadow_caster = nullptr;
ShadowSettings ShadowRenderer::m_settings;

void ShadowRenderer::initialize()
{
    m_shadow_fbo = make_unique<ShadowMapFrameBuffer>(m_settings.resolution, m_settings.resolution, "ShadowMap");
    m_shadow_ubo = make_unique<UniformBuffer>(SHADOW_DATA_SIZE, nullptr, SHADOW_DATA_UBO_BINDING, GL_DYNAMIC_DRAW);
    update_and_upload_data(ShadowGPUData{}); // zeroed = disabled
}

void ShadowRenderer::shutdown()
{
    m_shadow_caster = nullptr;
    m_shadow_fbo.reset();
    m_shadow_ubo.reset();
}

void ShadowRenderer::render()
{
    m_shadow_caster = SceneManager::get_current_scene()->get_all_lights()[0]; // Temporary. Will have to consider all lights in the scene.
    auto test = m_shadow_caster;

    const ShaderRef shader = ResourceManager::get_shader("shadow_depth");
    if (!m_shadow_caster || !shader || !shader->is_shader_compiled())
    {
        m_shadow_caster = nullptr;
        update_and_upload_data(ShadowGPUData{});
        return;
    }

    update_and_upload_data(build_shadow_data(*m_shadow_caster));
    render_depth_pass(shader);
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

void ShadowRenderer::update_and_upload_data(const ShadowGPUData& data)
{
    m_shadow_ubo->bind();
    m_shadow_ubo->bind_buffer_sub_data(&data, SHADOW_DATA_SIZE, 0);
    m_shadow_ubo->unbind();
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
