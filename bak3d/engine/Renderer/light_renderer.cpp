#include "light_renderer.h"

#include "shadow_renderer.h"
#include "Core/logger.h"

using namespace std;

unique_ptr<ShaderStorageBuffer> LightRenderer::m_lights_ssbo;
vector<LightGPUData> LightRenderer::m_staging_lights;
int LightRenderer::m_visible_light_count = 0;

namespace
{
    constexpr GLsizei INITIAL_LIGHT_CAPACITY = 256;
}

void LightRenderer::initialize()
{
    m_staging_lights.reserve(INITIAL_LIGHT_CAPACITY);
    m_lights_ssbo = make_unique<ShaderStorageBuffer>(0, nullptr, 15, GL_DYNAMIC_DRAW);
    B3D_LOG_INFO("LightManager initialized (SSBO capacity: %d lights).", INITIAL_LIGHT_CAPACITY);
}

void LightRenderer::shutdown()
{
    m_lights_ssbo.reset();
    m_staging_lights.clear();
}

void LightRenderer::update_and_upload_data(const std::vector<Light*>& active_lights, const Frustum& frustum, const glm::vec3& camera_position, bool allow_shadows)
{
    m_staging_lights.clear();

    // Cull
    vector<const Light*> visible_lights;
    visible_lights.reserve(active_lights.size());
    for (const Light* light : active_lights)
    {
        if (!light->is_active)
        {
            continue;
        }
        if (light->get_type() != LightType::Directional
            && !frustum.intersects_sphere(light->transform.get_global_position(), light->get_effective_radius()))
        {
            continue;
        }
        visible_lights.push_back(light);
    }

    // Pick shadow casters among the survivors (skipped for the asset preview scene)
    if (allow_shadows)
    {
        ShadowRenderer::select_casters(visible_lights, camera_position);
    }

    // Build payloads, tagging casters with their shadow layer
    for (const Light* light : visible_lights)
    {
        LightGPUData payload = light->get_light_gpu_data_payload();
        payload.shadow_index = allow_shadows ? ShadowRenderer::get_shadow_index(light) : -1;
        m_staging_lights.push_back(payload);
    }

    m_visible_light_count = static_cast<int>(m_staging_lights.size());
    if (m_staging_lights.empty())
    {
        return;
    }

    const auto required_size = static_cast<GLsizeiptr>(m_staging_lights.size()) * LIGHT_GPU_DATA_SIZE;
    m_lights_ssbo->bind();
    m_lights_ssbo->bind_buffer_data(m_staging_lights.data(), required_size);
    m_lights_ssbo->bind_to_binding_point(LIGHTS_SSBO_BINDING);
}
