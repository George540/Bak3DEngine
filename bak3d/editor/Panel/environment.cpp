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
#include "Renderer/shadow_renderer.h"
#include "Scene/Objects/light.h"

using namespace std;

namespace
{
    vector<const char*> m_skybox_mode_labels = { };
}

Environment::Environment() : EditorPanel("Environment")
{
    for (const SkyboxModeInfo& info : SkyboxModeInfos)
    {
        m_skybox_mode_labels.emplace_back(info.label);
    }
}

void Environment::begin_frame()
{
    EditorPanel::begin_frame();
}

void Environment::update()
{
    EditorPanel::update();

    draw_skybox_settings();

    ImGuiB3D::SeparatorWithSpacing(1);

    draw_rendering_settings();

    ImGuiB3D::SeparatorWithSpacing(1);

    draw_shadow_settings();

    ImGuiB3D::SeparatorWithSpacing(1);

    draw_post_processing_settings();

    ImGuiB3D::SeparatorWithSpacing(1);
}

void Environment::end_frame()
{
    EditorPanel::end_frame();
}

void Environment::draw_skybox_settings()
{
    bool skybox_enabled = GlobalSettings::get_global_setting_value<bool>(GlobalSettingOption::Skybox_Enabled);
    if (ImGuiB3D::PropertyToggle("Skybox", &skybox_enabled, "Render the skybox behind the scene."))
    {
        GlobalSettings::set_global_setting<bool>(GlobalSettingOption::Skybox_Enabled, skybox_enabled);
    }

    ImGui::BeginDisabled(!skybox_enabled);

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Skybox###SkyboxNode"))
    {
        // Mode
        int mode = GlobalSettings::get_global_setting_value<int>(GlobalSettingOption::Skybox_Mode);
        if (ImGuiB3D::PropertyDropdown("Mode", m_skybox_mode_labels, &mode, "Skybox rendering method.\nModes marked WIP are not implemented yet and draw nothing."))
        {
            GlobalSettings::set_global_setting<int>(GlobalSettingOption::Skybox_Mode, mode);
        }

        // Image selection
        if (static_cast<SkyboxMode>(mode) == SkyboxMode::Cubemap)
        {
            if (ResourceManager::Cubemaps.empty())
            {
                ImGui::TextDisabled("No cubemaps found in assets/skybox.");
            }
            else
            {
                const string cubemap_file_name = GlobalSettings::get_global_setting_value<string>(GlobalSettingOption::Skybox_Cubemap);
                string cubemap_name = FileLoader::get_name_from_filename(cubemap_file_name);
                const string before_pick = cubemap_name;

                ImTextureID preview_id = 0;
                if (ResourceManager::Cubemaps.contains(cubemap_name))
                {
                    preview_id = ImGuiB3D::GetAssetPreviewTextureId(ResourceManager::Cubemaps.get(cubemap_name).ref()->asset);
                }

                if (ImGuiB3D::PropertyImageButton("Cubemap", "Select the cubemap image used by the skybox.", preview_id, ImVec2(50.0f, 50.0f), ImVec2(0, 1), ImVec2(1, 0)))
                {
                    ImGui::OpenPopup("##picker_skybox_cubemap");
                }

                ImGuiB3D::AssetPickerPopup("##picker_skybox_cubemap", "Cubemaps", ResourceManager::Cubemaps, &cubemap_name);

                ImGui::SameLine();
                ImGui::Text("%s", cubemap_name.c_str());

                if (cubemap_name != before_pick)
                {
                    GlobalSettings::set_global_setting<string>(GlobalSettingOption::Skybox_Cubemap, cubemap_name);
                }
            }
        }

        // Rotation
        glm::vec3 rotation = GlobalSettings::get_global_setting_value<glm::vec3>(GlobalSettingOption::Skybox_Rotation);
        if (ImGuiB3D::PropertyDragFloat3("Rotation", &rotation, 0.1f, 0.0f, 0.0f, "%.3f", "Rotate the skybox based on Euler angles."))
        {
            GlobalSettings::set_global_setting<glm::vec3>(GlobalSettingOption::Skybox_Rotation, rotation);
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset##SkyboxRotation"))
        {
            GlobalSettings::set_global_setting<glm::vec3>(GlobalSettingOption::Skybox_Rotation, glm::vec3(0.0f));
        }

        // Color
        glm::vec3 color = GlobalSettings::get_global_setting_value<glm::vec3>(GlobalSettingOption::Skybox_Color);
        if (ImGuiB3D::ColorPicker3("Color", &color, "Multiplies the skybox color."))
        {
            GlobalSettings::set_global_setting<glm::vec3>(GlobalSettingOption::Skybox_Color, color);
        }

        // Intensity
        float intensity = GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::Skybox_Intensity);
        if (ImGuiB3D::PropertyDragFloat("Intensity", &intensity, 0.01f, 0.0f, 3.0f, "%.2f", "Brightness multiplier of the skybox."))
        {
            GlobalSettings::set_global_setting<float>(GlobalSettingOption::Skybox_Intensity, intensity);
        }

        ImGui::TreePop();
    }

    ImGui::EndDisabled();
}

void Environment::draw_rendering_settings()
{
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Editor Camera###EditorCameraNode"))
    {
        float near_plane = GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::EditorCamera_NearPlane);
        float far_plane = GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::EditorCamera_FarPlane);

        if (ImGuiB3D::PropertyDragFloat("Near Plane", &near_plane, 0.01f, 0.001f, 100.0f, "%.3f", "Near rendering distance."))
        {
            near_plane = glm::clamp(near_plane, 0.001f, far_plane - 0.01f);
            GlobalSettings::set_global_setting<float>(GlobalSettingOption::EditorCamera_NearPlane, near_plane);
        }
        if (ImGuiB3D::PropertyDragFloat("Far Plane", &far_plane, 1.0f, 0.1f, 10000.0f, "%.1f", "Far rendering distance."))
        {
            far_plane = glm::max(far_plane, near_plane + 0.01f);
            GlobalSettings::set_global_setting<float>(GlobalSettingOption::EditorCamera_FarPlane, far_plane);
        }
        if (ImGuiB3D::PropertyButton("Reset", "Reset Editor Camera", "Restore default near/far planes."))
        {
            GlobalSettings::set_global_setting<float>(GlobalSettingOption::EditorCamera_NearPlane, 0.1f);
            GlobalSettings::set_global_setting<float>(GlobalSettingOption::EditorCamera_FarPlane, 100.0f);
        }

        ImGui::TreePop();
    }
}

void Environment::draw_shadow_settings()
{
    ShadowSettings& shadow_settings = ShadowRenderer::get_settings();
    ImGuiB3D::PropertyToggle("Cast Shadows", &shadow_settings.cast_shadows, "Global toggle for processing shadows in the Renderer.");

    ImGui::BeginDisabled(!shadow_settings.cast_shadows);

    const bool modified = ShadowRenderer::is_modified();
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode(modified ? "Shadows (modified)###ShadowsNode" : "Shadows###ShadowsNode"))
    {
        ImGui::BeginDisabled(!modified);
        if (ImGuiB3D::PropertyButton("Reset", "Reset Shadows", "Restore all shadow settings to their defaults."))
        {
            ShadowRenderer::reset_settings();
        }
        ImGui::EndDisabled();

        int resolution_index = 2;
        for (int i = 0; i < 4; ++i)
        {
            if (resolution_values[i] == shadow_settings.resolution) { resolution_index = i; }
        }
        if (ImGuiB3D::PropertyDropdown("Resolution", resolution_labels, &resolution_index, "Shadow map size per light. Rebuilds the shadow map when changed."))
        {
            ShadowRenderer::apply_resolution(resolution_values[resolution_index]);
        }

        ImGui::SeparatorText("General");
        ImGuiB3D::PropertySliderFloat("Depth Bias", &shadow_settings.depth_bias_texels, 0.0f, 8.0f, "%.2f", "Constant bias in shadow texels.");
        ImGuiB3D::PropertySliderFloat("Slope Bias", &shadow_settings.slope_bias_texels, 0.0f, 8.0f, "%.2f", "Extra bias at grazing angles. Ignored when Receiver Plane Bias is on.");
        ImGuiB3D::PropertySliderFloat("Normal Offset", &shadow_settings.normal_offset_texels, 0.0f, 8.0f, "%.2f", "Pushes the lookup along the normal. Fixes acne without Peter Panning.");
        ImGuiB3D::PropertyToggle("Receiver Plane Bias", &shadow_settings.receiver_plane_bias, "Per-sample depth correction from the surface normal. Removes acne across wide PCSS kernels.");
        ImGuiB3D::PropertySliderFloat("Polygon Offset Factor", &shadow_settings.polygon_offset_factor, 0.0f, 8.0f, "%.2f", "Slope-scaled depth offset in the shadow depth pass.");
        ImGuiB3D::PropertySliderFloat("Polygon Offset Units", &shadow_settings.polygon_offset_units, 0.0f, 16.0f, "%.2f", "Constant depth offset in the shadow depth pass.");
        ImGuiB3D::PropertyToggle("Cull Front Faces", &shadow_settings.cull_front_faces, "Render back faces into the map. Single-sided planes and open meshes won't cast.");
        ImGuiB3D::PropertySliderFloat("Near Plane", &shadow_settings.near_plane, 0.01f, 5.0f, "%.2f", "Near plane of the light projection.");

        ImGui::SeparatorText("Filtering (PCSS)");
        ImGuiB3D::PropertyToggle("PCSS", &shadow_settings.pcss_enabled, "Contact-hardening soft shadows. When off, a fixed-radius disk filter is used.");
        ImGuiB3D::PropertySliderInt("Min Filter Radius", &shadow_settings.pcf_radius, 0, 4, "Minimum filter radius in shadow texels.");
        ImGuiB3D::PropertySliderFloat("Max Filter Radius", &shadow_settings.max_filter_radius_texels, 1.0f, 64.0f, "%.1f", "Caps penumbra width in texels. Bounds cost and noise.");
        ImGuiB3D::PropertySliderInt("Blocker Samples", &shadow_settings.blocker_samples, 1, 64, "Taps for the blocker search.");
        ImGuiB3D::PropertySliderInt("Filter Samples", &shadow_settings.filter_samples, 1, 128, "Taps for the final filter.");
        ImGuiB3D::PropertySliderFloat("Sun Angular Size", &shadow_settings.directional_angular_size_degrees, 0.0f, 10.0f, "%.2f", "Directional light angular diameter in degrees. Larger = softer penumbra.");
        ImGuiB3D::PropertySliderFloat("Spot Light Radius", &shadow_settings.spot_light_radius, 0.0f, 5.0f, "%.2f", "Spot light source radius in world units.");

        ImGui::TreePop();
    }
    ImGui::EndDisabled();
}

void Environment::draw_post_processing_settings()
{
    bool post_process_enabled = GlobalSettings::get_global_setting_value<bool>(GlobalSettingOption::PostProcessing_Enabled);
    ImGuiB3D::PropertyToggle("Post Processing", &post_process_enabled, "Toggle Post Processing.");
    GlobalSettings::set_global_setting<bool>(GlobalSettingOption::PostProcessing_Enabled, post_process_enabled);

    ImGui::BeginDisabled(!post_process_enabled);

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Post Processing"))
    {
        draw_color_grading_settings();
        draw_kernel_effect_settings();

        ImGui::TreePop();
    }

    ImGui::EndDisabled();
}

void Environment::draw_color_grading_settings()
{
    ImGui::SeparatorText("Color Grading");

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
}

void Environment::draw_kernel_effect_settings()
{
    ImGui::SeparatorText("Kernel Effect");
    
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
}

void Environment::draw_vignette_settings()
{
    ImGui::SeparatorText("Vignette");
    
    float vignette_intensity = GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::PostProcess_ColorGrading_VignetteIntensity);
    ImGuiB3D::PropertySliderFloat("Intensity", &vignette_intensity, -POST_PROCESS_COLORING_SLIDER_CLAMP, POST_PROCESS_COLORING_SLIDER_CLAMP, "%.1f", "Adjust vignette intensity. Positive intensity gives a darker vignette tone and negative intensity colors the inverse.");
    GlobalSettings::set_global_setting<float>(GlobalSettingOption::PostProcess_ColorGrading_VignetteIntensity, vignette_intensity);

    glm::vec4 vignette_color = GlobalSettings::get_global_setting_value<glm::vec4>(GlobalSettingOption::PostProcess_ColorGrading_VignetteColor);
    ImGuiB3D::ColorPicker4("Color", &vignette_color, "Adjust vignette coloring. Color gets inverted when intensity is negative.");
    GlobalSettings::set_global_setting<glm::vec4>(GlobalSettingOption::PostProcess_ColorGrading_VignetteColor, vignette_color);
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
