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

#include <string>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

constexpr const char* PARTICLE_EMITTER_DEFAULT_NAME = "Emitter_Instance";

/*
 * Per-instance data packed for GPU upload (tight, GPU-friendly)
 */
struct ParticleInstanceData
{
    glm::vec4 position; // world position (vec3) and billboard rotation in degrees (float)
    glm::vec4 color;    // rgba (alpha carries fade)
    float     scale;    // uniform scale
    float     _pad[3];  // std140 alignment
};
static_assert(sizeof(ParticleInstanceData) == 48, "ParticleInstanceData size mismatch"); // 3 * sizeof(vec4) = 3 * 16 = 48

/*
 * Editable runtime configuration handed to an emitter, so each emitter stores its own properties separately
 */
struct ParticleEmitterConfig
{
    // Sprite
    std::string texture_name = "particle_default.png";

    // Emission
    int max_particles = 50; // pool size; resize triggers IBO rebuild
    float emission_rate = 2.0f; // particles per second

    // Lifetime
    float lifetime = 2.0f;
    bool  randomize_lifetime = false;
    float lifetime_rand_offset = 0.5f; // lifetime +/- offset

    // Velocity
    glm::vec3 velocity = {0.0f, 1.0f, 0.0f};
    bool randomize_velocity = false;
    glm::vec3 velocity_rand_offset = {0.5f, 0.5f, 0.5f};

    // Scale
    float scale = 1.0f;
    bool randomize_scale = false;
    float scale_rand_offset = 0.1f;

    // Rotation
    float rotation = 0.0f;
    bool randomize_rotation = false;

    // Color
    glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
    bool randomize_color = false;

    // Behaviour
    bool fade_with_lifetime = false; // alpha = remaining_life / max_life
    float spawn_range = 1.0f; // XZ radius for instantiate scatter

    // Debug
    bool bounds_enabled = false;
};
