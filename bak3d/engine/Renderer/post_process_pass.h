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

#include <glm/glm.hpp>

#include "Buffers/buffer.h"
#include "Asset/resource_manager.h"
#include "Asset/shader.h"
#include "Core/global_definitions.h"

/*
 * Struct payload for general coloring post process settings
 */
struct PostProcessUniform_ColorGrading
{
    bool invert;
    bool grayscale;
    float brightness;
    float contrast;
    float hue;
    float saturation;
    float temperature;
    float vignette_intensity;
    glm::vec4 vignette_color;
};

/*
 * Base interface for all post process passes that share common properties.
 */
class PostProcessPass
{
public:
    PostProcessPass(std::string name, const std::string& shader_name)
        : m_shader(ResourceManager::get_shader(shader_name)), m_name(std::move(name)) {}

    virtual ~PostProcessPass() = default;

    virtual void process() = 0;

    ShaderRef get_shader() const { return m_shader; }
    const std::string& get_name() const { return m_name; }
    bool is_enabled() const{ return m_is_enabled; }

    void set_enabled(const bool enabled) { m_is_enabled = enabled; }
protected:
    ShaderRef m_shader;
    std::string m_name;
    bool m_is_enabled = false;
};

/*
 * Color Grading pass: invert, grayscale, brightness, saturation, contrast, etc.
 */
class PostProcessPass_ColorGrading : public PostProcessPass
{
public:
    PostProcessPass_ColorGrading()
        : PostProcessPass("ColorGrading", "color_grading") { m_is_enabled = true; }

    void process() override;
private:
    PostProcessUniform_ColorGrading m_post_process_color_grading = PostProcessUniform_ColorGrading();
};

/*
 * Color Grading pass: invert, grayscale, brightness, saturation, contrast, etc.
 */
class PostProcessPass_KernelEffect : public PostProcessPass
{
public:
    PostProcessPass_KernelEffect()
        : PostProcessPass("KernelEffect", "kernel_effect") { m_is_enabled = true; m_kernel_type = KernelEffectType::Sharpen; }

    PostProcessPass_KernelEffect(const KernelEffectType type)
        : PostProcessPass("KernelEffect", "kernel_effect") { m_is_enabled = true; m_kernel_type = type; }

    KernelEffectType get_kernel_type() const { return m_kernel_type; }

    void process() override;
private:
    KernelEffectType m_kernel_type;
};
