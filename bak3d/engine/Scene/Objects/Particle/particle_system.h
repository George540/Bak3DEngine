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

#include "particle_emitter.h"
#include "Scene/Objects/renderable_object.h"

/*
 * Handles all emitters and their particle data from an entire single renderable scene object.
 * Owns all the emitters and all buffer objects to upload data to the GPU from a single entry point.
 */
class ParticleSystem : public RenderableObject
{
public:
    explicit ParticleSystem(glm::vec3 position, const std::string& name = "ParticleSystem");
    ~ParticleSystem() override;

    void update(float dt) override;
    void draw() const override;

    ParticleEmitter* add_emitter(const std::string& name = PARTICLE_EMITTER_DEFAULT_NAME, const ParticleEmitterConfig& cfg = {});
    void remove_emitter(const std::string& name);
    void remove_last_emitter();
    ParticleEmitter* get_emitter(const std::string& name) const;
    const std::vector<std::unique_ptr<ParticleEmitter>>& get_emitters() const { return m_emitters; }

private:
    void ensure_ibo_capacity(const ParticleEmitter& emitter);
    bool emitter_name_exists(const std::string& name) const;

    // Per-emitter IBO map
    struct EmitterGPUData
    {
        std::unique_ptr<InstanceBuffer> ibo;
        std::unique_ptr<VertexArray> vao;
        int current_capacity = 0; // in particle count
    };
    std::unordered_map<std::string, EmitterGPUData> m_emitter_gpu;

    std::vector<std::unique_ptr<ParticleEmitter>> m_emitters;
};
