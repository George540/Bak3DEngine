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

#include <imgui_internal.h>
#include <ranges>

#include "imgui_b3d_extensions.h"
#include "Asset/resource_manager.h"
#include "Core/logger.h"
#include "Scene/scene.h"

using namespace std;

namespace
{
    constexpr ImVec2 IMAGE_BUTTON_PROPERTY_SIZE = ImVec2(40.0f, 40.0f);
    constexpr ImVec2 IMAGE_BUTTON_PROPERTY_SIZE_BORDERED = ImVec2(50.0f, 50.0f);
    constexpr ImVec2 POPUP_SIZE = ImVec2(200, 300);

    string asset_picker_item_id = "##sprite_picker";
    int object_selection_index = 0;
    int previous_object_selection_index = -1;
    vector<const char*> m_object_items = { "None", "Model", "Particle System", "Advanced Particles" };
    int model_selection_index = 0;
    vector<const char*> m_model_name_items = { "None" };

    Model* m_current_model = nullptr;
    ParticleSystem* m_current_particle_system = nullptr;

    unordered_map<aiTextureType, string> m_pending_texture_selections;

    void draw_property_button_selection_item(string* selected_name, const char* label, const char* tooltip_desc)
    {
        // Make popup ID as unique as possible to avoid duplicates
        asset_picker_item_id = "##" + *selected_name + label;
        const TextureRef current_tex = selected_name->empty()
            ? ResourceManager::get_texture("particle.png")
            : ResourceManager::get_texture(*selected_name);

        const ImTextureID preview_id = (current_tex && current_tex.is_valid())
            ? current_tex->get_texture_id()
            : 0;

        // Button that opens the picker
        if (ImGuiB3D::PropertyImageButton(label, tooltip_desc, preview_id, IMAGE_BUTTON_PROPERTY_SIZE))
        {
            const ImVec2 mouse_pos = ImGui::GetMousePos();
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            const ImVec2 work_pos  = viewport->WorkPos;
            const ImVec2 work_size = viewport->WorkSize;

            // Clamp based on viewport's work size and position.
            // Avoid popup being places outside the viewport's context.
            ImVec2 final_pos = mouse_pos;
            if (final_pos.x + POPUP_SIZE.x > work_pos.x + work_size.x)
            {
                final_pos.x = work_pos.x + work_size.x - POPUP_SIZE.x;
            }
            if (final_pos.y + POPUP_SIZE.y > work_pos.y + work_size.y)
            {
                final_pos.y = work_pos.y + work_size.y - POPUP_SIZE.y;
            }

            final_pos.x = ImMax(final_pos.x, work_pos.x);
            final_pos.y = ImMax(final_pos.y, work_pos.y);

            ImGui::SetNextWindowPos(final_pos);
            ImGui::OpenPopup(asset_picker_item_id.c_str());
        }

        ImGuiB3D::AssetPickerPopup(
            asset_picker_item_id.c_str(),
            "Sprite Selection",
            ResourceManager::Textures,
            selected_name
        );
    }

    void draw_texture_property_section(const MaterialRef& material, const string& texture_type_name, const aiTextureType texture_type, const string& parameter_name, float& surface_parameter)
    {
        string texture_pascal = texture_type_name;
        texture_pascal[0] = toupper(static_cast<unsigned char>(texture_pascal[0]));

        // Toggle
        const string use_key = "material.use_" + texture_type_name + "_texture";
        bool use_texture = material->has_uniform(use_key)
                           ? material->get_bool(use_key)
                           : false;
        ImGuiB3D::PropertyToggle(("Use " + texture_pascal + " Texture").c_str(), &use_texture);
        material->set_bool(use_key, use_texture);

        // Slider
        ImGui::BeginDisabled(use_texture);
        if (!parameter_name.empty())
        {
            const string slider_label = texture_pascal + " Slider";
            ImGuiB3D::PropertySliderFloat(slider_label.c_str(), &surface_parameter,
                                          0.0f, 1.0f, "%.3f");
        }
        ImGui::EndDisabled();

        // Texture Picker
        const string label_name = texture_pascal + " Texture";
        const bool has_texture  = material->has_texture_of_type(texture_type);

        string& pending = m_pending_texture_selections[texture_type];
        if (has_texture)
        {
            pending = material->get_texture_by_type(texture_type);
        }

        const string before_pick = pending;

        if (has_texture)
        {
            draw_property_button_selection_item(&pending, label_name.c_str(), nullptr);

            ImGui::SameLine();
            const string remove_id = "X##remove_" + texture_type_name;
            if (ImGui::Button(remove_id.c_str(), ImVec2(22, 22)))
            {
                material->remove_texture_by_type(texture_type);
                pending.clear(); // reset pending so the next frame shows "None"
            }
        }
        else
        {
            const string popup_id = "##asset_picker_none_" + texture_type_name;

            if (ImGuiB3D::PropertyButton(label_name.c_str(), "None", nullptr,
                                         IMAGE_BUTTON_PROPERTY_SIZE_BORDERED))
            {
                ImGui::OpenPopup(popup_id.c_str());
            }

            ImGuiB3D::AssetPickerPopup(
                popup_id.c_str(),
                (texture_pascal + " Selection").c_str(),
                ResourceManager::Textures,
                &pending);
        }

        // Write back if a new selection has been made
        if (pending != before_pick && !pending.empty())
        {
            material->set_texture_by_type(texture_type, pending);
        }

        ImGui::Spacing();
    }
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

    if (object_selection_index != previous_object_selection_index)
    {
        // Tear down whatever was previously active
        if (previous_object_selection_index == 1)
        {
            m_current_model = nullptr;
            Scene::instance->set_model(nullptr);
            model_selection_index = 0;
        }
        else if (previous_object_selection_index == 2)
        {
            m_current_particle_system = nullptr;
            Scene::instance->despawn_particle_system();
        }

        if (object_selection_index == 2)
        {
            m_current_particle_system = Scene::instance->spawn_particle_system();
        }

        previous_object_selection_index = object_selection_index;
    }

    if (object_selection_index == 1) // Model
    {
        draw_model_section();
    }
    else if (object_selection_index == 2) // Particle System
    {
        draw_particle_system_section();
    }
    else if (object_selection_index == 3) // Advanced Particles
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
            ImGui::Text("Edges:     %zu", m_current_model->get_unique_edges());
            ImGui::Text("Faces:     %zu", m_current_model->get_faces());
        }

        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Material Data"))
    {
        if (m_current_model)
        {
            const MaterialRef model_material = m_current_model->get_current_material();

            glm::vec4 surface_parameters = model_material->get_vec4("material.surface_parameters");

            draw_texture_property_section(model_material, "diffuse", aiTextureType_DIFFUSE, "material.surface_parameters.y", surface_parameters.y);
            draw_texture_property_section(model_material, "specular", aiTextureType_SPECULAR, "material.surface_parameters.z", surface_parameters.z);
            draw_texture_property_section(model_material, "normal", aiTextureType_NORMALS, "", surface_parameters.z);

            // Gamma Correction
            bool gamma_correction = model_material->get_bool("material.use_gamma_correction");
            ImGuiB3D::PropertyToggle("Gamma Correction", &gamma_correction, "Toggle between linear and gamma space for texture brightness.");
            model_material->set_bool("material.use_gamma_correction", gamma_correction);

            // Shininess
            ImGuiB3D::PropertySliderFloat("Shininess", &surface_parameters.w, 0.0f, 1.0f, "%.3f", "Control the shininess of the material.");

            // Ambient
            ImGuiB3D::PropertySliderFloat("Ambient", &surface_parameters.x, 0.0f, 1.0f, "%.3f");

            model_material->set_vec4("material.surface_parameters", surface_parameters);

            ImGuiB3D::MultiSpacing(3);

            // Apply Button
            if (ImGuiB3D::PropertyButton("Apply", "Save Changes", "Save material data if changed to corresponding material JSON file."))
            {
                model_material->save_to_file();
            }
        }

        ImGui::TreePop();
    }
}

void Details::draw_particle_system_section()
{
    if (m_current_particle_system)
    {
        int emitters_num = m_current_particle_system->get_emitters().size();
        ImGui::Text("Emitters (%d)", emitters_num);

        ImGui::SameLine();

        if (ImGui::Button("+", ImVec2(40, 0)))
        {
            m_current_particle_system->add_emitter();
        }

        ImGui::SameLine();

        if (ImGui::Button("-", ImVec2(40, 0)))
        {
            m_current_particle_system->remove_last_emitter();
        }

        for (auto& emitter : m_current_particle_system->get_emitters())
        {
            const string emitter_sub_label = emitter.get()->get_name();
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (ImGui::TreeNodeEx(emitter_sub_label.c_str(), ImGuiTreeNodeFlags_Framed))
            {
                draw_particle_emitter_section(*emitter);

                ImGui::TreePop();
            }

            ImGuiB3D::SeparatorWithSpacing();
        }
    }
    else
    {
        ImGui::TextUnformatted("No particle system object exists.");
    }
}

void Details::draw_particle_emitter_section(ParticleEmitter& emitter)
{
    auto& emitter_config = emitter.get_config();

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNodeEx("Metrics", ImGuiTreeNodeFlags_Framed))
    {
        ImGui::Text("Max Particles: %d", emitter.get_max_particles());
        ImGui::Text("Alive Particles: %d", emitter.get_alive_count());
        ImGui::Text("Dead Particles: %d", (emitter.get_max_particles() - emitter.get_alive_count()));
        ImGuiB3D::PropertyToggle("Bounds", &emitter_config.bounds_enabled, "Toggle bounds updating and drawing for debugging.");
        
        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNodeEx("Properties", ImGuiTreeNodeFlags_Framed))
    {
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Emission"))
        {
            // Max Particles
            ImGuiB3D::PropertySliderInt("Max Particles", &emitter_config.max_particles, emitter.get_config().emission_rate, 10000, "Control the maximum number of particles that can exist in the scene at once.\n"
                                                                                                                    "The maximum will be reached once emission rate matches the particle's lifetime.");
            // Emission Rate
            ImGuiB3D::PropertySliderFloat("Emission Rate", &emitter_config.emission_rate, 0.0f, emitter_config.max_particles, "%.2f", "Control the emission rate of the particle emitter.\n"
                                                                                                                    "Emission is calculated in particles per second (PPS).");
            ImGui::TreePop();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Lifetime"))
        {
            // Lifetime
            ImGuiB3D::PropertySliderFloat("Lifetime", &emitter_config.lifetime, 0.01f, 10.0f, "%.2f", "Control the particle's lifetime in seconds.");

            // Randomize Lifetime?
            ImGuiB3D::PropertyToggle("Randomize Lifetime", &emitter_config.randomize_lifetime, "Will particle lifetime be constant or randomized per particle?\n"
                                                                                                               "Control lifetime random offset below for more control.");
            // Lifetime Random Offset
            ImGui::BeginDisabled(!emitter_config.randomize_lifetime);
            ImGuiB3D::PropertySliderFloat("Lifetime Random Offset", &emitter_config.lifetime_rand_offset, 0.0f, emitter_config.lifetime, "%.2f", "Control the particle's lifetime random offset when activated.");
            ImGui::EndDisabled();

            ImGui::TreePop();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Velocity"))
        {
            // Velocity
            ImGuiB3D::PropertySliderFloat3("Velocity", &emitter_config.velocity, 0.0f, 10.0f, "%.2f", "Control the particle's velocity in all directions.");

            // Randomize Velocity? Simulate explosions
            ImGuiB3D::PropertyToggle("Randomize Velocity", &emitter_config.randomize_velocity, "Will particle velocity be constant or randomized?\n"
                                                                                                               "Control velocity random offset below for more control.");
            // Velocity Random Offset
            ImGui::BeginDisabled(!emitter_config.randomize_velocity);
            ImGuiB3D::PropertySliderFloat3("Velocity Random Offset", &emitter_config.velocity_rand_offset, 0.0f, 10.0f, "%.2f", "Control the particle's velocity random offset when activated.");
            ImGui::EndDisabled();

            ImGui::TreePop();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Scale"))
        {
            // Scale
            ImGuiB3D::PropertySliderFloat("Scale", &emitter_config.scale, 0.01f, 10.0f, "%.2f", "Control the particle's scale.");

            // Randomize Scale?
            ImGuiB3D::PropertyToggle("Randomize Scale", &emitter_config.randomize_scale, "Will particle scale be constant or randomized?\n"
                                                                                                               "Control scale random offset below for more control.");
            // Scale Random Offset
            ImGui::BeginDisabled(emitter_config.randomize_scale);
            ImGuiB3D::PropertySliderFloat("Scale Random Offset", &emitter_config.scale_rand_offset, 0.01f, emitter_config.scale, "%.2f", "Control the particle's scale random offset when activated.");
            ImGui::EndDisabled();

            ImGui::TreePop();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Rotation"))
        {
            // Randomize Rotation?
            ImGuiB3D::PropertyToggle("Randomize Rotation", &emitter_config.randomize_rotation, "Will particle rotation be constant or randomized?\n"
                                                                                                               "Rotation is randomized between 0 and 360 euler angle degrees.");

            // Rotation
            ImGui::BeginDisabled(emitter_config.randomize_rotation);
            ImGuiB3D::PropertySliderFloat("Rotation", &emitter_config.rotation, 0.0f, 360.0f, "%.2f", "Control the particle's rotation.");
            ImGui::EndDisabled();

            ImGui::TreePop();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Color"))
        {
            // Randomize Color?
            ImGuiB3D::PropertyToggle("Randomize Color", &emitter_config.randomize_color, "Will particle color be constant or randomized?\n"
                                                                                                               "Control scale random offset below for more control.");
            // Color
            ImGui::BeginDisabled(emitter_config.randomize_color);
            ImGuiB3D::PropertyColorPicker("Color", &emitter_config.color, "Control particle color in normalized RGBA channel.");
            ImGui::EndDisabled();

            // Sprite
            string selected_sprite_name =  emitter.get_texture() ? emitter.get_texture()->get_file_name() : "particle_default.png";

            draw_property_button_selection_item(&selected_sprite_name, "Sprite", "Select particle sprite for current emitter.");

            emitter.set_texture(ResourceManager::get_texture(selected_sprite_name));

            ImGui::TreePop();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Behaviour"))
        {
            // Fade with lifetime?
            ImGuiB3D::PropertyToggle("Fade with Lifetime", &emitter_config.fade_with_lifetime, "Will particle fade with lifetime?\nIf yes, particle will be discarded once opacity reaches zero.");

            // Spawn Range within bounding volume
            ImGuiB3D::PropertySliderFloat("Spawn Range", &emitter_config.spawn_range, 0.0f, 10.0f, "%.2f", "Control the spawning range within the particle system's bounding volume.");

            ImGui::TreePop();
        }

        ImGui::TreePop();
    }
}