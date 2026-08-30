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

#include "environment.h"

#include "imgui_b3d_extensions.h"
#include "Asset/resource_manager.h"
#include "Core/global_settings.h"
#include "Input/event_manager.h"
#include "Input/renderdoc_manager.h"
#include "Renderer/renderer.h"
#include "Scene/Objects/light.h"

using namespace std;

namespace
{
    vector<string> m_msaa_samples = { };
    vector<string> m_light_type_items = { };
}

Environment::Environment() : EditorPanel("Environment")
{
    // Dynamically create MSAA sampling options for dropdown selection based on hardware's max samples.
    const int msaa_max_samples = Renderer::get_msaa_frame_buffer()->get_samples();
    m_msaa_samples.reserve(msaa_max_samples);
    for (int sample_id = 2; sample_id <= msaa_max_samples; sample_id *= 2)
    {
        string label = to_string(sample_id) + "x" + to_string(sample_id);
        m_msaa_samples.push_back(label);
    }

    // Could be hardcoded since we know the light types already, but let's keep it modular.
    constexpr int light_types_num = static_cast<int>(LightType::Max);
    m_light_type_items.reserve(light_types_num);
    for (int light_type_id = 0; light_type_id < light_types_num; light_type_id++)
    {
        const LightType light_type = static_cast<LightType>(light_type_id);
        string label = light_type_to_string(light_type);
        m_light_type_items.push_back(label);
    }
}

void Environment::begin_frame()
{
    EditorPanel::begin_frame();
}

void Environment::update()
{
    EditorPanel::update();

    draw_tools_settings();

    ImGuiB3D::SeparatorWithSpacing(1);

    draw_general_settings();

    ImGuiB3D::SeparatorWithSpacing(1);

    draw_post_processor_settings();

    ImGuiB3D::SeparatorWithSpacing(1);
}

void Environment::end_frame()
{
    EditorPanel::end_frame();
}

void Environment::draw_tools_settings()
{
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("GPU & Display"))
    {
        // Force Fail
        bool force_fail = GlobalSettings::get_global_setting_value<bool>(GlobalSettingOption::Resources_ForceFail);
        ImGuiB3D::PropertyToggle("Force Fail Hot-Reload", &force_fail, "Force shaders to output visual results even if they fail during compilation or hot-reload");
        GlobalSettings::set_global_setting<bool>(GlobalSettingOption::Resources_ForceFail, force_fail);
        
        // Hot-Reload Shaders
        if (ImGuiB3D::PropertyButton("Hot Reload", "Hot Reload Shaders", "Hot-Reload shaders after updating shader files after running sync_shaders.py script."))
        {
            ResourceManager::reload_shaders();
        }

        // Trigger Capture
        if (ImGuiB3D::PropertyButton("Trigger Capture", "Trigger GPU Capture", "Trigger GPU frame capture if RenderDoc app is available. Make sure it's installed locally first!"))
        {
            RenderDocManager::trigger_capture();
        }

        // VSync
        bool vsync = GlobalSettings::get_global_setting_value<bool>(GlobalSettingOption::Vsync);
        if (ImGuiB3D::PropertyToggle("VSync", &vsync, "Toggle VSync. If enabled, GLFW frame rate synchronizes with display's refresh rate."))
        {
            EventManager::toggle_vsync(vsync);
            GlobalSettings::set_global_setting<bool>(GlobalSettingOption::Vsync, vsync);
        }

        ImGui::TreePop();
    }
}

void Environment::draw_general_settings()
{
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("General"))
    {
        // Toggle background color
        glm::vec4 bg_color_vec4 = GlobalSettings::get_global_setting_value<glm::vec4>(GlobalSettingOption::BackgroundColor);
        ImGuiB3D::ColorPicker4("Background Color", &bg_color_vec4, "Change background color using glClearColor(...)");
        GlobalSettings::set_global_setting<glm::vec4>(GlobalSettingOption::BackgroundColor, bg_color_vec4);

        ImGui::TreePop();
    }
}

void Environment::draw_post_processor_settings()
{
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Effects"))
    {
        draw_rasterization_settings();
        draw_post_processing_settings();

        ImGui::TreePop();
    }
}

void Environment::draw_rasterization_settings()
{
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Rasterization"))
    {
        bool msaa_enabled = GlobalSettings::get_global_setting_value<bool>(GlobalSettingOption::AA_MSAA_Enabled);
        ImGuiB3D::PropertyToggle("MSAA", &msaa_enabled, "Toggle Multisample Anti-Aliasing");
        GlobalSettings::set_global_setting<bool>(GlobalSettingOption::AA_MSAA_Enabled, msaa_enabled);

        ImGui::BeginDisabled(!msaa_enabled);
        {
            int msaa_sample = GlobalSettings::get_global_setting_value<int>(GlobalSettingOption::AA_MSAA_Samples);
            const string preview = to_string(msaa_sample) + "x" + to_string(msaa_sample);
            if (ImGuiB3D::PropertyBeginDropdown("MSAA Samples", preview.c_str(), "Controls MSAA quality."))
            {
                for (int n = 0; n < m_msaa_samples.size(); n++)
                {
                    ImGui::PushID(n);

                    // Bitwise shift to power of two:
                    // Index 0 -> 2 (2^1)
                    // Index 1 -> 4 (2^2)
                    // Index 2 -> 8 (2^3)...
                    const int sample_value = 1 << (n + 1);
                    const bool is_selected = (msaa_sample == sample_value);

                    if (ImGui::Selectable(m_msaa_samples[n].c_str(), is_selected))
                    {
                        msaa_sample = sample_value;
                        GlobalSettings::set_global_setting<int>(GlobalSettingOption::AA_MSAA_Samples, msaa_sample);
                    }

                    if (is_selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }

                    ImGui::PopID();
                }

                ImGui::EndCombo();
            }
        }
        ImGui::EndDisabled();

        ImGuiB3D::SeparatorWithSpacing(1);

        ImGui::TreePop();
    }
}

void Environment::draw_post_processing_settings()
{
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Post Processing"))
    {
        bool post_process_enabled = GlobalSettings::get_global_setting_value<bool>(GlobalSettingOption::PostProcessing_Enabled);
        ImGuiB3D::PropertyToggle("Enabled", &post_process_enabled, "Toggle Post Processing.");
        GlobalSettings::set_global_setting<bool>(GlobalSettingOption::PostProcessing_Enabled, post_process_enabled);

        ImGui::BeginDisabled(!post_process_enabled);
        {
            draw_color_grading_settings();
            draw_kernel_effect_settings();
        }
        ImGui::EndDisabled();

        ImGuiB3D::SeparatorWithSpacing(1);

        ImGui::TreePop();
    }
}

void Environment::draw_color_grading_settings()
{
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Color Grading"))
    {
        if (ImGuiB3D::PropertyButton("Reset", "Reset Color Grading", "Reset Post Processing effects to disabled values."))
        {
            reset_color_grading_to_defaults();
        }
        
        bool invert = GlobalSettings::get_global_setting_value<bool>(GlobalSettingOption::PostProcess_ColorGrading_Invert);
        ImGuiB3D::PropertyToggle("Invert", &invert, "Invert color image.");
        GlobalSettings::set_global_setting<bool>(GlobalSettingOption::PostProcess_ColorGrading_Invert, invert);

        bool grayscale = GlobalSettings::get_global_setting_value<bool>(GlobalSettingOption::PostProcess_ColorGrading_Grayscale);
        ImGuiB3D::PropertyToggle("Grayscale", &grayscale, "Turn color image into black and white using Perceptive Luminance.");
        GlobalSettings::set_global_setting<bool>(GlobalSettingOption::PostProcess_ColorGrading_Grayscale, grayscale);

        float brightness = GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::PostProcess_ColorGrading_Brightness);
        ImGuiB3D::PropertySliderFloat("Brightness", &brightness, -POST_PROCESS_COLORING_SLIDER_CLAMP, POST_PROCESS_COLORING_SLIDER_CLAMP, "%.1f", "Adjust color image's brightness levels");
        GlobalSettings::set_global_setting<float>(GlobalSettingOption::PostProcess_ColorGrading_Brightness, brightness);

        float contrast = GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::PostProcess_ColorGrading_Contrast);
        ImGuiB3D::PropertySliderFloat("Contrast", &contrast, -POST_PROCESS_COLORING_SLIDER_CLAMP, POST_PROCESS_COLORING_SLIDER_CLAMP, "%.1f", "Adjust color image's contrast.");
        GlobalSettings::set_global_setting<float>(GlobalSettingOption::PostProcess_ColorGrading_Contrast, contrast);

        float hue = GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::PostProcess_ColorGrading_Hue);
        ImGuiB3D::PropertySliderFloat("Hue", &hue, -POST_PROCESS_COLORING_SLIDER_CLAMP, POST_PROCESS_COLORING_SLIDER_CLAMP, "%.1f", "Adjust color image's hue.");
        GlobalSettings::set_global_setting<float>(GlobalSettingOption::PostProcess_ColorGrading_Hue, hue);

        float saturation = GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::PostProcess_ColorGrading_Saturation);
        ImGuiB3D::PropertySliderFloat("Saturation", &saturation, -POST_PROCESS_COLORING_SLIDER_CLAMP, POST_PROCESS_COLORING_SLIDER_CLAMP, "%.1f", "Adjust color image's saturation.");
        GlobalSettings::set_global_setting<float>(GlobalSettingOption::PostProcess_ColorGrading_Saturation, saturation);

        float temperature = GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::PostProcess_ColorGrading_Temperature);
        ImGuiB3D::PropertySliderFloat("Temperature", &temperature, -POST_PROCESS_COLORING_SLIDER_CLAMP, POST_PROCESS_COLORING_SLIDER_CLAMP, "%.1f", "Adjust color image's temperature. Controls red and blue color channels.");
        GlobalSettings::set_global_setting<float>(GlobalSettingOption::PostProcess_ColorGrading_Temperature, temperature);

        ImGuiB3D::SeparatorWithSpacing(1);

        draw_vignette_settings();

        ImGui::TreePop();
    }
}

void Environment::draw_kernel_effect_settings()
{
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Kernel Effects - Convolution Matrices"))
    {
        if (ImGuiB3D::PropertyButton("Reset", "Reset Kernel Effects", "Reset kernel effects to disabled values."))
        {
            reset_kernel_effects_to_defaults();
        }

        float sharpen_intensity = GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::PostProcess_KernelEffect_SharpenIntensity);
        ImGuiB3D::PropertySliderFloat("Sharpen", &sharpen_intensity, 0.0f, 1.0f, "%.1f", "Adjust Sharpen intensity. Enhances edges and details by increasing the contrast between adjacent pixels.");
        GlobalSettings::set_global_setting<float>(GlobalSettingOption::PostProcess_KernelEffect_SharpenIntensity, sharpen_intensity);

        float sobel_intensity = GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::PostProcess_KernelEffect_SobelIntensity);
        ImGuiB3D::PropertySliderFloat("Sobel", &sobel_intensity, 0.0f, 1.0f, "%.1f", "Adjust Sobel Edge Detection intensity. Bright colored pixels are marked as edges, black for transitions.");
        GlobalSettings::set_global_setting<float>(GlobalSettingOption::PostProcess_KernelEffect_SobelIntensity, sobel_intensity);

        float emboss_intensity = GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::PostProcess_KernelEffect_EmbossIntensity);
        ImGuiB3D::PropertySliderFloat("Emboss", &emboss_intensity, 0.0f, 1.0f, "%.1f", "Adjust Embossing intensity. Emboss is a monochrome that highlights edges with bright color for transitions and dark for edges.");
        GlobalSettings::set_global_setting<float>(GlobalSettingOption::PostProcess_KernelEffect_EmbossIntensity, emboss_intensity);

        float box_blur_intensity = GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::PostProcess_KernelEffect_BoxBlurIntensity);
        ImGuiB3D::PropertySliderFloat("Box Blur", &box_blur_intensity, 0.0f, 1.0f, "%.1f", "Adjust Box Blur intensity. Reduce image details by averaging high contrast edges.");
        GlobalSettings::set_global_setting<float>(GlobalSettingOption::PostProcess_KernelEffect_BoxBlurIntensity, box_blur_intensity);

        float laplacian_intensity = GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::PostProcess_KernelEffect_LaplacianIntensity);
        ImGuiB3D::PropertySliderFloat("Laplacian", &laplacian_intensity, 0.0f, 1.0f, "%.1f", "Adjust Laplacian intensity. Used mostly for edge detection and image denoising.");
        GlobalSettings::set_global_setting<float>(GlobalSettingOption::PostProcess_KernelEffect_LaplacianIntensity, laplacian_intensity);

        ImGui::TreePop();
    }
}

void Environment::draw_vignette_settings()
{
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Vignette"))
    {
        float vignette_intensity = GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::PostProcess_ColorGrading_VignetteIntensity);
        ImGuiB3D::PropertySliderFloat("Intensity", &vignette_intensity, -POST_PROCESS_COLORING_SLIDER_CLAMP, POST_PROCESS_COLORING_SLIDER_CLAMP, "%.1f", "Adjust vignette intensity. Positive intensity gives a darker vignette tone and negative intensity colors the inverse.");
        GlobalSettings::set_global_setting<float>(GlobalSettingOption::PostProcess_ColorGrading_VignetteIntensity, vignette_intensity);

        glm::vec4 vignette_color = GlobalSettings::get_global_setting_value<glm::vec4>(GlobalSettingOption::PostProcess_ColorGrading_VignetteColor);
        ImGuiB3D::ColorPicker4("Color", &vignette_color, "Adjust vignette coloring. Color gets inverted when intensity is negative.");
        GlobalSettings::set_global_setting<glm::vec4>(GlobalSettingOption::PostProcess_ColorGrading_VignetteColor, vignette_color);

        ImGuiB3D::SeparatorWithSpacing(1);

        ImGui::TreePop();
    }
}

void Environment::reset_color_grading_to_defaults()
{
    GlobalSettings::set_global_setting<bool>(GlobalSettingOption::PostProcess_ColorGrading_Invert, false);
    GlobalSettings::set_global_setting<bool>(GlobalSettingOption::PostProcess_ColorGrading_Grayscale, false);
    GlobalSettings::set_global_setting<float>(GlobalSettingOption::PostProcess_ColorGrading_Brightness, 0.0f);
    GlobalSettings::set_global_setting<float>(GlobalSettingOption::PostProcess_ColorGrading_Contrast, 0.0f);
    GlobalSettings::set_global_setting<float>(GlobalSettingOption::PostProcess_ColorGrading_Hue, 0.0f);
    GlobalSettings::set_global_setting<float>(GlobalSettingOption::PostProcess_ColorGrading_Saturation, 0.0f);
    GlobalSettings::set_global_setting<float>(GlobalSettingOption::PostProcess_ColorGrading_Temperature, 0.0f);
    GlobalSettings::set_global_setting<float>(GlobalSettingOption::PostProcess_ColorGrading_VignetteIntensity, 0.0f);
    GlobalSettings::set_global_setting<float>(GlobalSettingOption::PostProcess_ColorGrading_VignetteColor, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
}

void Environment::reset_kernel_effects_to_defaults()
{
    GlobalSettings::set_global_setting<float>(GlobalSettingOption::PostProcess_KernelEffect_SharpenIntensity, 0.0f);
    GlobalSettings::set_global_setting<float>(GlobalSettingOption::PostProcess_KernelEffect_SobelIntensity, 0.0f);
    GlobalSettings::set_global_setting<float>(GlobalSettingOption::PostProcess_KernelEffect_EmbossIntensity, 0.0f);
    GlobalSettings::set_global_setting<float>(GlobalSettingOption::PostProcess_KernelEffect_BoxBlurIntensity, 0.0f);
    GlobalSettings::set_global_setting<float>(GlobalSettingOption::PostProcess_KernelEffect_LaplacianIntensity, 0.0f);
}
