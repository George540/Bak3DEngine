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

#include <random>

#include "Asset/resource_manager.h"

using namespace std;

namespace
{
    // Cheap and safe way to get a random float value between a given range
    float rand_float(const float min, const float max)
    {
        thread_local mt19937 rng(random_device{}());
        uniform_real_distribution dist(min, max);
        return dist(rng);
    }
}

#include "particle_emitter.h"

ParticleEmitter::ParticleEmitter(const string& name, ParticleEmitterConfig config)
    : m_name(name)
    , m_config(std::move(config))
{
    resize_pool(m_config.max_particles);
    m_prev_max_particles = m_config.max_particles;

    m_texture = ResourceManager::get_texture(m_config.texture_name);

    m_bounds = new BoundingBox();
}

ParticleEmitter::~ParticleEmitter() = default;

void ParticleEmitter::update(float dt, const glm::vec3& emitter_world_pos)
{
    if (!m_enabled)
    {
        return;
    }

    // Check if max_particles changed in config
    if (m_config.max_particles != m_prev_max_particles)
    {
        resize_pool(m_config.max_particles);
        m_prev_max_particles = m_config.max_particles;
    }

    const int pool_size = static_cast<int>(m_current_lifetime.size());

    for (int i = 0; i < pool_size; ++i)
    {
        if (m_current_lifetime[i] <= 0.f)
        {
            continue; // skip dead ones
        }

        m_current_lifetime[i] -= dt;

        if (m_current_lifetime[i] <= 0.f)
        {
            kill_one_particle(i);
            continue;
        }

        m_position_x[i] += m_velocity_x[i] * dt;
        m_position_y[i] += m_velocity_y[i] * dt;
        m_position_z[i] += m_velocity_z[i] * dt;

        // Fade alpha proportional to remaining lifetime
        if (m_config.fade_with_lifetime)
        {
            m_color_a[i] = (m_current_lifetime[i] / m_max_lifetime[i]) * m_config.color.a;
        }
        else
        {
            m_color_a[i] = m_config.color.a;
        }

        // Sync color rgb from config
        if (!m_config.randomize_color)
        {
            m_color_r[i] = m_config.color.r;
            m_color_g[i] = m_config.color.g;
            m_color_b[i] = m_config.color.b;
        }
        if (!m_config.randomize_scale)
        {
            m_scale[i] = m_config.scale;
        }
        if (!m_config.randomize_rotation)
        {
            m_rotation[i] = m_config.rotation;
        }
    }

    // Emit new particles
    m_spawn_accumulator += m_config.emission_rate * dt;
    const int to_spawn = static_cast<int>(m_spawn_accumulator);
    m_spawn_accumulator -= static_cast<float>(to_spawn);

    for (int s = 0; s < to_spawn; ++s)
    {
        spawn_one_particle(emitter_world_pos);
    }

    // Pack GPU instance data. GPU clips dead particles (alpha == 0)
    for (int i = 0; i < pool_size; ++i)
    {
        m_instance_data[i].position = {m_position_x[i], m_position_y[i], m_position_z[i], m_rotation[i]};
        m_instance_data[i].scale = m_scale[i];
        m_instance_data[i].color = {m_color_r[i], m_color_g[i], m_color_b[i], m_color_a[i]};
    }

    update_debug_bounds(dt);
}

void ParticleEmitter::draw() const
{
    if (m_config.bounds_enabled)
    {
        m_bounds->draw();
    }
}

void ParticleEmitter::apply_config(const ParticleEmitterConfig& cfg)
{
    const bool needs_resize = (cfg.max_particles != m_config.max_particles);
    m_config = cfg;

    if (needs_resize)
    {
        resize_pool(m_config.max_particles);
        m_prev_max_particles = m_config.max_particles;
    }

    // Reload texture if name changed
    m_texture = ResourceManager::get_texture(m_config.texture_name);
}

void ParticleEmitter::resize_pool(int new_max)
{
    // Resize every array:
    //     Shrinking: particles beyond new_max are lost.
    //     Growing: new slots are dead (lifetime = 0) and added to dead_indices.

    const int old_max = static_cast<int>(m_current_lifetime.size());

    m_position_x.resize(new_max, 0.f);
    m_position_y.resize(new_max, 0.f);
    m_position_z.resize(new_max, 0.f);

    m_velocity_x.resize(new_max, 0.f);
    m_velocity_y.resize(new_max, 0.f);
    m_velocity_z.resize(new_max, 0.f);

    m_color_r.resize(new_max, 1.f);
    m_color_g.resize(new_max, 1.f);
    m_color_b.resize(new_max, 1.f);
    m_color_a.resize(new_max, 0.f); // start invisible / dead

    m_current_lifetime.resize(new_max, 0.f);
    m_max_lifetime.resize(new_max, 1.f);

    m_scale.resize(new_max, 1.f);
    m_rotation.resize(new_max, 0.f);

    m_instance_data.resize(new_max);

    m_dead_indices.clear();
    m_dead_indices.reserve(new_max);

    // Rebuild dead index stack:
    //     Keep existing dead indices that are still in range, then add new slots.
    //     Erase indices that are now out of range (shrink case).
    {
        vector<int> valid_dead;
        valid_dead.reserve(m_dead_indices.size());
        for (int index : m_dead_indices)
        {
            if (index < new_max)
            {
                valid_dead.push_back(index);
            }
        }

        // New slots (growing case): they start dead
        for (int i = old_max; i < new_max; ++i)
        {
            valid_dead.push_back(i);
        }

        // Recalculate alive count from lifetime array
        // (simpler than tracking what got lost during shrink)
        m_alive_count = 0;
        for (int i = 0; i < new_max; ++i)
        {
            if (m_current_lifetime[i] > 0.f)
            {
                ++m_alive_count;
            }
        }

        m_dead_indices = move(valid_dead);
    }
}

void ParticleEmitter::spawn_one_particle(const glm::vec3& emitter_world_pos)
{
    if (m_dead_indices.empty())
    {
        return; // pool is full
    }

    const int index = m_dead_indices.back();
    m_dead_indices.pop_back();

    // Position: scatter in XZ around emitter
    m_position_x[index] = emitter_world_pos.x + rand_float(-m_config.spawn_range, m_config.spawn_range);
    m_position_y[index] = emitter_world_pos.y;
    m_position_z[index] = emitter_world_pos.z + rand_float(-m_config.spawn_range, m_config.spawn_range);

    // Velocity
    if (m_config.randomize_velocity)
    {
        m_velocity_x[index] = rand_float(-m_config.velocity_rand_offset.x, m_config.velocity_rand_offset.x);
        m_velocity_y[index] = rand_float(-m_config.velocity_rand_offset.y, m_config.velocity_rand_offset.y);
        m_velocity_z[index] = rand_float(-m_config.velocity_rand_offset.z, m_config.velocity_rand_offset.z);
    }
    else
    {
        m_velocity_x[index] = m_config.velocity.x;
        m_velocity_y[index] = m_config.velocity.y;
        m_velocity_z[index] = m_config.velocity.z;
    }

    // Lifetime
    const float lt = m_config.randomize_lifetime
        ? m_config.lifetime + rand_float(-m_config.lifetime_rand_offset, m_config.lifetime_rand_offset)
        : m_config.lifetime;
    m_current_lifetime[index] = glm::max(lt, 0.01f); // guard against zero/negative
    m_max_lifetime[index] = m_current_lifetime[index];

    // Scale
    m_scale[index] = m_config.randomize_scale
        ? glm::max(m_config.scale - rand_float(0.f, m_config.scale_rand_offset), 0.001f)
        : m_config.scale;

    // Rotation
    m_rotation[index] = m_config.randomize_rotation
        ? rand_float(0.f, 360.f)
        : m_config.rotation;

    // Color
    if (m_config.randomize_color)
    {
        m_color_r[index] = rand_float(0.f, 1.f);
        m_color_g[index] = rand_float(0.f, 1.f);
        m_color_b[index] = rand_float(0.f, 1.f);
    }
    else
    {
        m_color_r[index] = m_config.color.r;
        m_color_g[index] = m_config.color.g;
        m_color_b[index] = m_config.color.b;
    }
    m_color_a[index] = m_config.color.a;

    ++m_alive_count;
}

void ParticleEmitter::kill_one_particle(const int index)
{
    m_current_lifetime[index] = 0.f;
    m_color_a[index] = 0.f; // invisible (to be GPU culled)
    m_dead_indices.push_back(index);
    --m_alive_count;
}

void ParticleEmitter::update_debug_bounds(const float dt)
{
    if (m_config.bounds_enabled)
    {
        glm::vec3 bounds_scale;
        if (m_config.randomize_velocity)
        {
            m_bounds->transform.set_local_position(glm::vec3(0.0f, 0.0f, 0.0f));
            bounds_scale = (glm::vec3(m_config.velocity_rand_offset) * m_config.lifetime) + (m_config.spawn_range * 2.0f);
        }
        else
        {
            const auto horizontal_scale = m_config.spawn_range * 2.0f;
            m_bounds->transform.set_local_position(glm::vec3(0.0f, 1.0f, 0.0f));
            bounds_scale = (glm::vec3(m_config.velocity) * m_config.lifetime) + glm::vec3(horizontal_scale, 0.0f, horizontal_scale);
        }
        m_bounds->transform.set_local_scale(bounds_scale);
        m_bounds->update(dt);
    }
}
