#include "particle_system.h"

#include "Asset/resource_manager.h"

#include "Core/logger.h"

using namespace std;

ParticleSystem::ParticleSystem(const string& name)
    : RenderableObject(ResourceManager::get_material("particle"),
                glm::vec3(0.0f),
                        name)
{
    // Build shared quad geometry (unit quad in 0..1 range, shader centres it)
    m_vbo = new VertexBuffer(static_cast<GLsizei>(QUAD_VERTICES.size()) * VEC4_SIZE, QUAD_VERTICES.data());
    m_ebo = new ElementBuffer(static_cast<GLsizei>(QUAD_INDICES.size()) * UINT_SIZE, QUAD_INDICES.data());

    // Add one emitter by default
    add_emitter();

    B3D_LOG_INFO("Particle System '%s' created.", name.c_str());
}

ParticleSystem::~ParticleSystem()
{
    m_emitters.clear();
    m_emitter_gpu.clear();
}

void ParticleSystem::ensure_ibo_capacity(const ParticleEmitter& emitter)
{
    const string& emitter_name = emitter.get_name();
    const int needed = emitter.get_max_particles();
    auto& gpu_data = m_emitter_gpu[emitter_name];

    if (gpu_data.current_capacity >= needed)
    {
        return;
    }

    const GLsizei byte_size = static_cast<GLsizei>(sizeof(ParticleInstanceData)) * needed;

    // Allocate new IBO
    gpu_data.ibo = std::make_unique<InstanceBuffer>(byte_size, nullptr, GL_DYNAMIC_DRAW);
    gpu_data.current_capacity = needed;

    // (Re)build VAO — fully configure it once here, never again until next resize
    gpu_data.vao = std::make_unique<VertexArray>();
    gpu_data.vao->bind();

    // Attribute 0: quad vertex from the shared VBO
    // VBO must be bound when glVertexAttribPointer is called so VAO records it
    m_vbo->bind();
    gpu_data.vao->set_attrib_pointer(0, 4, GL_FLOAT, GL_FALSE, VEC4_SIZE, nullptr);

    // Attributes 1-4: per-instance data from this emitter's IBO
    gpu_data.ibo->bind();
    constexpr GLsizei stride = sizeof(ParticleInstanceData);
    gpu_data.vao->set_attrib_pointer(1, 4, GL_FLOAT, GL_FALSE, stride,
        reinterpret_cast<void*>(offsetof(ParticleInstanceData, position)), 1);
    gpu_data.vao->set_attrib_pointer(2, 4, GL_FLOAT, GL_FALSE, stride,
        reinterpret_cast<void*>(offsetof(ParticleInstanceData, color)), 1);
    gpu_data.vao->set_attrib_pointer(3, 1, GL_FLOAT, GL_FALSE, stride,
        reinterpret_cast<void*>(offsetof(ParticleInstanceData, scale)), 1);

    m_ebo->bind();
    m_vbo->unbind();
    gpu_data.vao->unbind();
}

bool ParticleSystem::emitter_name_exists(const std::string& name) const
{
    return ranges::any_of(m_emitters, [&name](const unique_ptr<ParticleEmitter>& e){ return e->get_name() == name; });
}

void ParticleSystem::update(const float dt)
{
    if (!m_visible)
    {
        return;
    }

    for (auto& emitter : m_emitters)
    {
        emitter->update(dt, m_position);
        ensure_ibo_capacity(*emitter);

        // Upload instance data
        auto& gpu_data = m_emitter_gpu[emitter->get_name()];
        const auto& instance_data = emitter->get_instance_data();

        gpu_data.ibo->bind();
        gpu_data.ibo->bind_buffer_sub_data(
            instance_data.data(),
            sizeof(ParticleInstanceData) * emitter->get_max_particles(),
            0);
        gpu_data.ibo->unbind();
    }
}

void ParticleSystem::draw() const
{
    if (!m_visible)
    {
        return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    (*m_material_slot)->set_int("sprite", 0);
    RenderableObject::draw();

    for (const auto& emitter : m_emitters)
    {
        if (!emitter->is_enabled() || emitter->get_alive_count() == 0)
        {
            continue;
        }

        auto it = m_emitter_gpu.find(emitter->get_name());
        if (it == m_emitter_gpu.end() || !it->second.ibo || !it->second.vao)
        {
            continue;
        }

        // Single bind — all attribute state already recorded in this VAO
        it->second.vao->bind();

        const TextureRef& texture = emitter->get_texture();
        if (texture) texture->bind(0);

        glDrawElementsInstanced(
            GL_TRIANGLES,
            static_cast<GLsizei>(QUAD_INDICES.size()),
            GL_UNSIGNED_INT,
            nullptr,
            emitter->get_max_particles() // GPU clips dead particles via alpha = 0
        );

        emitter->draw();

        if (texture)
        {
            Texture2D::unbind();
        }

        it->second.vao->unbind();
    }

    (*m_material_slot)->get_shader()->unuse();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
}

ParticleEmitter* ParticleSystem::add_emitter(const string& name, const ParticleEmitterConfig& cfg)
{
    // Guard against duplicate names
    string unique_name = name;
    if (emitter_name_exists(unique_name))
    {
        int suffix = 1;
        do
        {
            unique_name = name + "_" + to_string(suffix);
            ++suffix;
        }
        while (emitter_name_exists(unique_name));

        B3D_LOG_INFO("ParticleSystem: emitter name '%s' already exists, renamed to '%s'.", name.c_str(), unique_name.c_str());
    }

    auto emitter = make_unique<ParticleEmitter>(unique_name, cfg);
    ParticleEmitter* raw = emitter.get();
    m_emitters.push_back(std::move(emitter));

    // Ensure GPU slot exists immediately
    m_emitter_gpu[unique_name]; // default-constructs EmitterGPUData
    ensure_ibo_capacity(*raw);

    B3D_LOG_INFO("ParticleSystem: emitter '%s' added (%d max particles).", unique_name.c_str(), cfg.max_particles);

    return raw;
}

void ParticleSystem::remove_emitter(const string& name)
{
    const auto it = ranges::find_if(m_emitters,
                                    [&name](const unique_ptr<ParticleEmitter>& e){ return e->get_name() == name; });

    if (it == m_emitters.end())
    {
        B3D_LOG_WARNING("ParticleSystem: emitter '%s' not found for removal.", name.c_str());
        return;
    }

    m_emitters.erase(it);
    m_emitter_gpu.erase(name);
    B3D_LOG_INFO("ParticleSystem: emitter '%s' removed.", name.c_str());
}

void ParticleSystem::remove_last_emitter()
{
    if (m_emitters.empty())
    {
        B3D_LOG_WARNING("ParticleSystem: no emitters to remove.");
        return;
    }

    const std::string name = m_emitters.back()->get_name();
    m_emitters.pop_back();
    m_emitter_gpu.erase(name);

    B3D_LOG_INFO("ParticleSystem: last emitter '%s' removed.", name.c_str());
}

ParticleEmitter* ParticleSystem::get_emitter(const string& name) const
{
    for (const auto& emitter : m_emitters)
    {
        if (emitter->get_name() == name)
        {
            return emitter.get();
        }
    }
    return nullptr;
}
