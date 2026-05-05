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

#include "details.h"

#include <ranges>

#include "imgui_b3d_extensions.h"
#include "Asset/resource_manager.h"
#include "Core/logger.h"
#include "Scene/scene.h"

using namespace std;

namespace
{
    constexpr ImVec2 IMAGE_BUTTON_PROPERTY_SIZE = ImVec2(40.0f, 40.0f);
    int object_selection_index = 0;
    vector<const char*> m_object_items = { "None", "Model", "Particle System", "Advanced Particles" };
    int model_selection_index = 0;
    vector<const char*> m_model_name_items = { "None" };
    int render_selection_index = 0;
    vector<const char*> m_map_selection_items = { "Full Render", "Albedo", "Specular", "Normal" };

    Model* m_current_model = nullptr;
}

Details::Details() : EditorPanel("Details")
{
    for (const auto& model_name : ResourceManager::Models.all() | views::keys)
    {
        m_model_name_items.push_back(model_name.c_str());
    }
}

void Details::begin_frame()
{
    EditorPanel::begin_frame();
}

void Details::update()
{
    EditorPanel::update();

    draw_object_section();

    if (object_selection_index == 1) // Model
    {
        draw_model_section();
    }
    else if (object_selection_index == 2) // Particle System
    {
        draw_particle_system_section();
    }
    else if (object_selection_index == 2) // Advanced Particles
    {
        ImGui::TextUnformatted("New GPU Particle system on the way!");
    }
    else
    {
        ImGui::TextDisabled("No object currently selected to inspect. Select in the dropdown above.");
    }
}

void Details::end_frame()
{
    EditorPanel::end_frame();
}

void Details::draw_object_section()
{
    // Object Selection
    if (ImGuiB3D::PropertyDropdown("Object Selection", m_object_items, &object_selection_index, "Select an object type to render in the scene"))
    {
        B3D_LOG_INFO("Selected object: %s", m_object_items[object_selection_index]);
    }
}

void Details::draw_model_section()
{
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Model Data"))
    {
        ImGuiB3D::PropertyDropdown("Model", m_model_name_items, &model_selection_index, "Select one of the loaded asset models to render in the scene.");
        const string selected_model_name = m_model_name_items[model_selection_index];
        Model* selected_model = model_selection_index > 0 ? ResourceManager::get_model(selected_model_name).ref()->asset : nullptr;
        m_current_model = Scene::instance->get_model();

        bool is_model_different = selected_model && m_current_model ? selected_model->get_object_name() != m_current_model->get_object_name() : selected_model != m_current_model;
        if (is_model_different)
        {
            m_current_model = selected_model;
            Scene::instance->set_model(m_current_model);
        }

        if (m_current_model)
        {
            ImGui::Text("Vertices:  %zu", m_current_model->get_vertices());
            ImGui::Text("Edges:     %zu",m_current_model->get_unique_edges().size());
            ImGui::Text("Faces:     %zu", m_current_model->get_faces());
        }

        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Material Data"))
    {
        if (m_current_model)
        {
            // Albedo
            if (m_current_model->has_texture_of_type(aiTextureType_DIFFUSE))
            {
                const ImTextureID albedo = m_current_model->textures_cache[aiTextureType_DIFFUSE]->get_texture_id();
                ImGuiB3D::PropertyImageButton("Albedo", nullptr, albedo, ImVec2(40.0f, 40.0f));
            }
            else
            {
                ImGuiB3D::PropertyButton("Albedo", "None", nullptr, ImVec2(50.0f, 50.0f));
            }

            // Specular
            if (m_current_model->has_texture_of_type(aiTextureType_SPECULAR))
            {
                const ImTextureID specular = m_current_model->textures_cache[aiTextureType_SPECULAR]->get_texture_id();
                ImGuiB3D::PropertyImageButton("Specular", nullptr, specular, ImVec2(40.0f, 40.0f));
            }
            else
            {
                ImGuiB3D::PropertyButton("Specular", "None", nullptr, ImVec2(50.0f, 50.0f));
            }

            // Normal
            if (m_current_model->has_texture_of_type(aiTextureType_HEIGHT))
            {
                const ImTextureID normals = m_current_model->textures_cache[aiTextureType_HEIGHT]->get_texture_id();
                ImGuiB3D::PropertyImageButton("Normals", nullptr, normals, ImVec2(40.0f, 40.0f));
            }
            else
            {
                ImGuiB3D::PropertyButton("Normals", "None", nullptr, ImVec2(50.0f, 50.0f));
            }

            // Gamma Correction
            bool gamma_correction = m_current_model->gamma_correction;
            ImGuiB3D::PropertyToggle("Gamma Correction", &gamma_correction, "Toggle between linear and gamma space for texture brightness.");
            m_current_model->gamma_correction = gamma_correction;
        }

        ImGui::TreePop();
    }
}

void Details::draw_particle_system_section()
{
    
}
