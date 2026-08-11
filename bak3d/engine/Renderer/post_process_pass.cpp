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

#include "post_process_pass.h"

#include "Core/global_settings.h"

namespace
{
    float normalize_value(const float raw_value, const float min, const float max, const float new_min = -1.0f, const float new_max = 1.0f)
    {
        return (raw_value - min) / (max - min) * (new_max - new_min) + new_min;
    }
}
void PostProcessPass_ColorGrading::process()
{
    if (!m_is_enabled)
    {
        return;
    }
    
    m_post_process_color_grading.invert = GlobalSettings::get_global_setting_value<bool>(GlobalSettingOption::PostProcess_ColorGrading_Invert);
    m_post_process_color_grading.grayscale = GlobalSettings::get_global_setting_value<bool>(GlobalSettingOption::PostProcess_ColorGrading_Grayscale);
    m_post_process_color_grading.brightness = normalize_value(GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::PostProcess_ColorGrading_Brightness), -POST_PROCESS_COLORING_SLIDER_CLAMP, POST_PROCESS_COLORING_SLIDER_CLAMP);
    m_post_process_color_grading.contrast = normalize_value(GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::PostProcess_ColorGrading_Contrast), -POST_PROCESS_COLORING_SLIDER_CLAMP, POST_PROCESS_COLORING_SLIDER_CLAMP);
    m_post_process_color_grading.hue = normalize_value(GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::PostProcess_ColorGrading_Hue), -POST_PROCESS_COLORING_SLIDER_CLAMP, POST_PROCESS_COLORING_SLIDER_CLAMP);
    m_post_process_color_grading.saturation = normalize_value(GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::PostProcess_ColorGrading_Saturation), -POST_PROCESS_COLORING_SLIDER_CLAMP, POST_PROCESS_COLORING_SLIDER_CLAMP);
    m_post_process_color_grading.temperature = normalize_value(GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::PostProcess_ColorGrading_Temperature), -POST_PROCESS_COLORING_SLIDER_CLAMP, POST_PROCESS_COLORING_SLIDER_CLAMP);
    m_post_process_color_grading.vignette_intensity = GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::PostProcess_ColorGrading_VignetteIntensity); // No need to normalize value for shader. Keep raw value.
    m_post_process_color_grading.vignette_color = GlobalSettings::get_global_setting_value<glm::vec4>(GlobalSettingOption::PostProcess_ColorGrading_VignetteColor);

    m_shader->set_bool("post_process_color_grading.invert", m_post_process_color_grading.invert);
    m_shader->set_bool("post_process_color_grading.grayscale", m_post_process_color_grading.grayscale);
    m_shader->set_float("post_process_color_grading.brightness", m_post_process_color_grading.brightness);
    m_shader->set_float("post_process_color_grading.contrast", m_post_process_color_grading.contrast);
    m_shader->set_float("post_process_color_grading.hue", m_post_process_color_grading.hue);
    m_shader->set_float("post_process_color_grading.saturation", m_post_process_color_grading.saturation);
    m_shader->set_float("post_process_color_grading.temperature", m_post_process_color_grading.temperature);
    m_shader->set_float("post_process_color_grading.vignette_intensity", m_post_process_color_grading.vignette_intensity);
    m_shader->set_vec4("post_process_color_grading.vignette_color", m_post_process_color_grading.vignette_color);
}

void PostProcessPass_KernelEffect::process()
{
    if (!m_is_enabled)
    {
        return;
    }

    switch (m_kernel_type)
    {
        case KernelEffectType::Sharpen:
            m_shader->set_float_array("kernel_matrix", STANDARD_SHARPEN.data(), 9);
            m_shader->set_float("kernel_intensity", GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::PostProcess_KernelEffect_SharpenIntensity));
            break;
        case KernelEffectType::Sobel:
            m_shader->set_float_array("kernel_matrix", SOBEL_EDGE_DETECTION.data(), 9);
            m_shader->set_float("kernel_intensity", GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::PostProcess_KernelEffect_SobelIntensity));
            break;
        case KernelEffectType::Emboss:
            m_shader->set_float_array("kernel_matrix", EMBOSS.data(), 9);
            m_shader->set_float("kernel_intensity", GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::PostProcess_KernelEffect_EmbossIntensity));
            break;
        case KernelEffectType::BoxBlur:
            m_shader->set_float_array("kernel_matrix", BOX_BLUR.data(), 9);
            m_shader->set_float("kernel_intensity", GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::PostProcess_KernelEffect_BoxBlurIntensity));
            break;
        case KernelEffectType::Laplacian:
            m_shader->set_float_array("kernel_matrix", LAPLACIAN_SHARPEN.data(), 9);
            m_shader->set_float("kernel_intensity", GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::PostProcess_KernelEffect_LaplacianIntensity));
            break;
        case KernelEffectType::Max:
            break;
        default:
            break;
    }
}
