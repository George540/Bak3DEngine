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

using namespace std;

static constexpr GLuint NUM_ELEMENTS = 128;
static constexpr GLuint WORKGROUP_SIZE = 64;

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

    // Single SSBO: written by the compute shader as a storage block, read by the vertex shader as a vertex attribute source.
    m_ssbo_in = make_unique<ShaderStorageBuffer>(NUM_ELEMENTS * VEC4_SIZE, nullptr, 16);

    // Describe the SAME buffer object to the VAO via GL_ARRAY_BUFFER.
    // This is independent of its GL_SHADER_STORAGE_BUFFER binding at index 16 -
    // a buffer object can be bound to multiple targets at once.
    m_vao->bind();
    m_ssbo_in->bind();
    m_vao->set_attrib_pointer(0, 4, GL_FLOAT, GL_FALSE, VEC4_SIZE, nullptr);
    m_vao->unbind();
    m_ssbo_in->unbind();

    B3D_LOG_INFO("Advanced Particle System '%s' created.", name.c_str());
}

AdvancedParticleSystem::~AdvancedParticleSystem()
{
    
}

void AdvancedParticleSystem::update(float dt)
{
    if (!m_comp_test_shader || !m_comp_test_shader->is_shader_compiled())
    {
        return;
    }

    m_comp_test_shader->use();
    m_ssbo_in->bind_to_binding_point(16);
    m_comp_test_shader->dispatch_compute_1d(NUM_ELEMENTS, WORKGROUP_SIZE);
    Buffer::insert_memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
    m_comp_test_shader->unuse();
}

void AdvancedParticleSystem::draw() const
{
    if (!has_material())
    {
        return;
    }

    (*m_material_slot)->get_shader()->use();

    glEnable(GL_PROGRAM_POINT_SIZE);

    m_vao->bind();
    glDrawArrays(GL_POINTS, 0, NUM_ELEMENTS);
    m_vao->unbind();

    glDisable(GL_PROGRAM_POINT_SIZE);
}
