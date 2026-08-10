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

#include "AdvancedParticleSystem.h"

#include "Asset/resource_manager.h"
#include "Input/event_manager.h"
#include "Renderer/debug_scope.h"

using namespace std;

static constexpr GLuint NUM_ELEMENTS = 2000000;

AdvancedParticleSystem::AdvancedParticleSystem(const std::string& name)
    : RenderableObject(ResourceManager::get_material("particle_advanced"),
                glm::vec3(0.0f),
                        name)
{
    m_comp_test_shader = ResourceManager::get_shader("particle_advanced_emit");
    if (!m_comp_test_shader || !m_comp_test_shader->is_shader_compiled())
    {
        B3D_LOG_ERROR("Compute shader missing or failed to compile.");
    }

    m_ssbo_in = make_unique<ShaderStorageBuffer>(NUM_ELEMENTS * VEC4_SIZE, nullptr, 16);

    m_vao->bind();
    m_ssbo_in->bind_to_target(GL_ARRAY_BUFFER);
    m_vao->set_attrib_pointer(0, 4, GL_FLOAT, GL_FALSE, VEC4_SIZE, nullptr);
    m_ssbo_in->unbind_from_target(GL_ARRAY_BUFFER);
    m_vao->unbind();

    B3D_LOG_INFO("Advanced Particle System '%s' created.", name.c_str());
}

void AdvancedParticleSystem::update(float dt)
{
    return;
}

void AdvancedParticleSystem::draw() const
{
    simulate();

    if (!has_material())
    {
        return;
    }

    const ShaderRef particle_shader = (*m_material_slot)->get_shader();
    if (!particle_shader || !particle_shader->is_shader_compiled())
    {
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        return;
    }
    particle_shader->use();
    particle_shader->set_float("viewport_height", static_cast<float>(EventManager::get_viewport_height()));
    (*m_material_slot)->set_float("point_scale", 1.0f);
    apply_material();

    glEnable(GL_PROGRAM_POINT_SIZE);

    m_vao->bind();
    glDrawArrays(GL_POINTS, 0, NUM_ELEMENTS);
    m_vao->unbind();
}

void AdvancedParticleSystem::simulate() const
{
    if (!m_comp_test_shader || !m_comp_test_shader->is_shader_compiled())
    {
        return;
    }

    DebugScopeGroup scope("GPU Particles: Emit Compute Dispatch");

    m_comp_test_shader->use();
    m_comp_test_shader->set_int("particle_count", NUM_ELEMENTS); // @TODO: Add to UBO
    m_comp_test_shader->set_float("spawn_extent", 5.0f);
    m_ssbo_in->bind_to_binding_point(16);
    m_comp_test_shader->dispatch_compute_1d(NUM_ELEMENTS, WORK_GROUP_LOCAL_SIZE);
    Buffer::insert_memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
    m_comp_test_shader->unuse();
}
