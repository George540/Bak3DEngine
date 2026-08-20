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

#include "Asset/mesh_data.h"
#include "Asset/resource_manager.h"
#include "Input/event_manager.h"
#include "Renderer/debug_scope.h"

using namespace std;

static constexpr GLuint PARTICLE_INITIAL_COUNT = 30000000;
static constexpr GLuint PARTICLE_COMMAND_SLOT = 0; // single batch for now (expand for multiple emitters later)
static constexpr GLuint EMIT_WORK_GROUP_COUNT = (PARTICLE_INITIAL_COUNT + WORK_GROUP_LOCAL_SIZE - 1) / WORK_GROUP_LOCAL_SIZE;

AdvancedParticleSystem::AdvancedParticleSystem(const std::string& name)
    : RenderableObject(ResourceManager::get_material("particle_advanced"),
                glm::vec3(0.0f),
                        name)
{
    m_emit_compute_shader = ResourceManager::get_shader("particle_advanced_emit");
    if (!m_emit_compute_shader || !m_emit_compute_shader->is_shader_compiled())
    {
        B3D_LOG_ERROR("Emit compute shader missing or failed to compile.");
    }

    m_simulate_compute_shader = ResourceManager::get_shader("particle_advanced_simulate");
    if (!m_simulate_compute_shader || !m_simulate_compute_shader->is_shader_compiled())
    {
        B3D_LOG_ERROR("Simulate compute shader missing or failed to compile.");
    }

    m_positions_ssbo = make_unique<ShaderStorageBuffer>(PARTICLE_INITIAL_COUNT * VEC4_SIZE, nullptr, 16);

    m_draw_indirect_command_buffer = make_unique<IndirectCommandBuffer>(1, IndirectCommandType::DrawArraysIndirect, "IndirectCommandBuffer - Advanced Particle System - Draw");
    m_draw_indirect_command_buffer->set_command(PARTICLE_COMMAND_SLOT, DrawArraysIndirectCommand{ PARTICLE_INITIAL_COUNT, 1, 0, 0 });

    m_dispatch_emit_indirect_command_buffer = make_unique<IndirectCommandBuffer>(1, IndirectCommandType::DispatchIndirect, "IndirectCommandBuffer - Advanced Particles System - Compute Emit+Simulate");
    m_dispatch_emit_indirect_command_buffer->set_command(PARTICLE_COMMAND_SLOT, DispatchIndirectCommand{EMIT_WORK_GROUP_COUNT, 1, 1});

    AdvancedParticleSystem::emit();

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
        return;
    }
    particle_shader->use();
    particle_shader->set_float("viewport_height", static_cast<float>(EventManager::get_viewport_height()));
    apply_material();

    glEnable(GL_PROGRAM_POINT_SIZE);

    (*m_mesh_slot)->get_vao()->bind();
    m_draw_indirect_command_buffer->draw_command(GL_POINTS, PARTICLE_COMMAND_SLOT);
    (*m_mesh_slot)->get_vao()->unbind();

    glDisable(GL_PROGRAM_POINT_SIZE);
}

void AdvancedParticleSystem::emit() const
{
    if (!m_emit_compute_shader || !m_emit_compute_shader->is_shader_compiled())
    {
        return;
    }

    DebugScopeGroup scope("GPU Particles: Emit Compute Dispatch");

    m_emit_compute_shader->use();
    m_emit_compute_shader->set_int("particle_count", PARTICLE_INITIAL_COUNT); // @TODO: Add to UBO
    m_emit_compute_shader->set_float("spawn_extent", 10.0f);
    m_positions_ssbo->bind_to_binding_point(16);
    m_dispatch_emit_indirect_command_buffer->dispatch_command(PARTICLE_COMMAND_SLOT);
    Buffer::insert_memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT);
    m_emit_compute_shader->unuse();
}

void AdvancedParticleSystem::simulate() const
{
    if (!m_simulate_compute_shader || !m_simulate_compute_shader->is_shader_compiled())
    {
        return;
    }

    DebugScopeGroup scope("GPU Particles: Simulate Compute Dispatch");

    m_simulate_compute_shader->use();
    m_simulate_compute_shader->set_int("particle_count", PARTICLE_INITIAL_COUNT);
    m_simulate_compute_shader->set_float("dt", EventManager::get_frame_time());
    m_positions_ssbo->bind_to_binding_point(16);
    m_dispatch_emit_indirect_command_buffer->dispatch_command(PARTICLE_COMMAND_SLOT);
    Buffer::insert_memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT);
    m_simulate_compute_shader->unuse();
}
