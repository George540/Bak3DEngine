#include "light_renderer.h"

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

void LightRenderer::update_and_upload_data(const std::vector<Light*>& active_lights, const Frustum& frustum)
{
    m_staging_lights.clear();

    for (const auto& light : active_lights)
    {
        if (!light->is_active)
        {
            continue;
        }

        if (light->get_type() != LightType::Directional)
        {
            const glm::vec3 position = light->transform.get_global_position();
            if (!frustum.intersects_sphere(position, light->get_effective_radius()))
            {
                continue;
            }
        }

        m_staging_lights.push_back(light->get_light_gpu_data_payload());

        m_visible_light_count = static_cast<int>(m_staging_lights.size());
        if (m_staging_lights.empty())
        {
            return;
        }

        // Grows/reallocates automatically when the requested size differs from the buffer's current size
        const auto required_size = static_cast<GLsizeiptr>(m_staging_lights.size()) * LIGHT_GPU_DATA_SIZE;
        m_lights_ssbo->bind();
        m_lights_ssbo->bind_buffer_data(m_staging_lights.data(), required_size);
        m_lights_ssbo->bind_to_binding_point(LIGHTS_SSBO_BINDING);
    }
}
