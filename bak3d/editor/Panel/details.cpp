
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
#include "Asset/model.h"
#include "Asset/resource_manager.h"
#include "Core/logger.h"
#include "Scene/scene.h"
#include "Scene/scene_manager.h"
#include "Scene/Objects/advanced_particle_system.h"
#include "Scene/Objects/Particle/particle_system.h"

using namespace std;

namespace
{
    constexpr ImVec2 IMAGE_BUTTON_PROPERTY_SIZE = ImVec2(40.0f, 40.0f);
    constexpr ImVec2 IMAGE_BUTTON_PROPERTY_SIZE_BORDERED = ImVec2(50.0f, 50.0f);
    constexpr ImVec2 POPUP_SIZE = ImVec2(200, 300);

    unordered_map<aiTextureType, string> m_pending_texture_selections;
    string asset_picker_item_id = "##sprite_picker";

    vector<string> m_light_type_items = { };

    char selected_object_name_buffer[64] = "";

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

void Details::begin_frame()
{
    EditorPanel::begin_frame();
}

void Details::update()
{
    EditorPanel::update();

    draw_object();
}

void Details::end_frame()
{
    EditorPanel::end_frame();
}

void Details::draw_object()
{
    if (SceneObject* selected_object = SceneManager::get_current_scene()->get_selected_scene_object())
    {
        draw_scene_object_section(selected_object);

        if (const auto selected_renderable = dynamic_cast<RenderableObject*>(selected_object))
        {
            ImGuiB3D::SeparatorWithSpacing();

            draw_renderable_object_section(selected_renderable);

            if (selected_renderable->get_object_type() == SceneObjectType::Light)
            {
                ImGuiB3D::SeparatorWithSpacing();

                draw_light_section(dynamic_cast<Light*>(selected_renderable));
            }
            else if (selected_renderable->get_object_type() == SceneObjectType::ParticleSystem)
            {
                ImGuiB3D::SeparatorWithSpacing();

                draw_particle_system_section(dynamic_cast<ParticleSystem*>(selected_renderable));
            }
        }

        ImGui::EndDisabled();
    }
    else
    {
        ImGui::TextDisabled("No object currently selected to inspect. Select from the Scene Graph to view details.");
    }
}

void Details::draw_scene_object_section(SceneObject* selected_object)
{
    ImGui::Checkbox("##Active", &selected_object->is_active);

    ImGui::SameLine();
    
    ImGui::PushID(selected_object);

    // Safely sync the buffer with the object name when there is no active input
    if (!ImGui::IsItemActive())
    {
        strncpy_s(selected_object_name_buffer, selected_object->get_object_name().c_str(), sizeof(selected_object_name_buffer) - 1);
        selected_object_name_buffer[sizeof(selected_object_name_buffer) - 1] = '\0';
    }
    
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::InputText("##object_name", selected_object_name_buffer, IM_ARRAYSIZE(selected_object_name_buffer), ImGuiInputTextFlags_EnterReturnsTrue))
    {
        selected_object->set_object_name(string(selected_object_name_buffer));
    }

    ImGui::PopID();

    ImGuiB3D::SeparatorWithSpacing();

    ImGui::BeginDisabled(!selected_object->is_active);
    
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Transform"))
    {
        // Position
        glm::vec3 position = selected_object->transform.get_local_position();
        if (ImGuiB3D::PropertyDragFloat3("Position", &position, 0.1f, 0.0f, 0.0f, "%.3f", "Translate scene object."))
        {
            selected_object->transform.set_local_position(position);
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset##Position"))
        {
            selected_object->transform.set_local_position(glm::vec3(0.0f));
        }

        // Euler Rotation
        glm::vec3 rotation = selected_object->transform.get_local_euler_rotation();
        if (ImGuiB3D::PropertyDragFloat3("Rotation", &rotation, 0.1f, 0.0f, 0.0f, "%.3f", "Rotate scene object based on Euler angles."))
        {
            selected_object->transform.set_local_euler_rotation(rotation);
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset##Rotation"))
        {
            selected_object->transform.set_local_euler_rotation(glm::vec3(0.0f));
        }

        // Scale
        glm::vec3 scale = selected_object->transform.get_local_scale();
        if (ImGuiB3D::PropertyDragFloat3("Scale", &scale, 0.1f, 0.0f, 0.0f, "%.3f", "Scale scene object."))
        {
            selected_object->transform.set_local_scale(scale);
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset##Scale"))
        {
            selected_object->transform.set_local_scale(glm::vec3(1.0f));
        }

        ImGui::TreePop();
    }
}

void Details::draw_renderable_object_section(RenderableObject* selected_renderable)
{
    bool is_visible = selected_renderable->is_visible();
    ImGuiB3D::PropertyToggle("Visible", &is_visible, "Whether object gets drawn every frame.");
    selected_renderable->set_visible(is_visible);

    // @TODO: Make them selectable with an image button?
    if (selected_renderable->get_material())
    {
        ImGui::Text("Material: %s", selected_renderable->get_material()->get_object_name().c_str());
    }
    if (selected_renderable->get_mesh())
    {
        ImGui::Text("Mesh: %s", selected_renderable->get_mesh()->get_object_name().c_str());
    }
}

void Details::draw_light_section(Light* light)
{
    if (!light)
    {
        return;
    }

    // Light Type
    const LightType light_type = light->get_type();
    if (ImGuiB3D::PropertyBeginDropdown("Type", light_type_to_string(light_type), "Select Light Caster type. Choices are:\n"
                                                                                                                  " - Directional\n"
                                                                                                                  " - Point\n"
                                                                                                                  " - Spot\n"
                                                                                                                  " - Area"))
    {
        for (int light_type_index = 0; light_type_index < m_light_type_items.size(); light_type_index++)
        {
            ImGui::PushID(light_type_index);

            const bool is_selected = (light_type_to_string(light_type) == m_light_type_items[light_type_index].c_str());

            if (ImGui::Selectable(m_light_type_items[light_type_index].c_str(), is_selected))
            {
                light->set_type(static_cast<LightType>(light_type_index));
            }

            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }

            ImGui::PopID();
        }

        ImGui::EndCombo();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Coloration"))
    {
        // Intensity
        float intensity = light->get_intensity();
        if (ImGuiB3D::PropertyDragFloat("Intensity", &intensity, 0.1f, 0.001f, 0.0f, "%.3f", "Intensity of the light"))
        {
            light->set_intensity(intensity);
        }

        // Color
        glm::vec3 light_color = light->get_diffuse();
        if (ImGuiB3D::ColorPicker3("Color", &light_color, "Change light's color"))
        {
            light->set_diffuse(light_color);
        }

        ImGui::TreePop();
    }

    // Unique light type properties
    if (light_type == LightType::Point || light_type == LightType::Spot)
    {
        const string light_type_string = light_type_to_string(light_type);
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode(light_type_string.c_str()))
        {
            // Attenuation Radius
            float attenuation = light->get_attenuation_radius();
            if (ImGuiB3D::PropertySliderFloat("Attenuation Radius", &attenuation, 0.0f, 300.0f, "%.1f", "Alter attenuation radius of the light. Controls linear and quadratic factors of attenuation calculation."))
            {
                light->set_attenuation(attenuation);
            }

            if (light_type == LightType::Spot)
            {
                // Cone size
                float cone_size = light->get_cone_size();
                if (ImGuiB3D::PropertySliderFloat("Cone Size", &cone_size, 1.0f, 180.0f, "%.1f", "Alter cone angle of the spotlight. The greater the size, the greater the light casting area."))
                {
                    light->set_cone_size(cone_size);
                }

                // Inner Cone Angle
                bool cone_angles_changed = false;
                float inner = light->get_cone_angle_inner_cutoff();
                float outer = light->get_cone_angle_outer_cutoff();
                cone_angles_changed |= ImGuiB3D::PropertySliderFloat("Inner Cone Angle", &inner, 1.0f, 170.0f, "%.1f", "Alter inner cone angle cutoff of the spotlight. The larger the value, the smoother the cutoff.");
                cone_angles_changed |= ImGuiB3D::PropertySliderFloat("Outer Cone Angle", &outer, inner, 180.0f, "%.1f", "Alter outer cone angle cutoff of the spotlight. The larger the value, the smoother the cutoff.");
                if (cone_angles_changed)
                {
                    light->set_cone_angles(inner, outer);
                }
            }

            ImGui::TreePop();
        }
    }
}

void Details::draw_model_section()
{
    /*ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Model Data"))
    {
        ImGuiB3D::PropertyDropdown("Model", m_model_name_items, &model_selection_index, "Select one of the loaded asset models to render in the scene.");
        const string selected_model_name = m_model_name_items[model_selection_index];
        Model* selected_model = model_selection_index > 0 ? ResourceManager::get_model(selected_model_name).ref()->asset : nullptr;
        //m_current_model = Scene::instance->get_model();

        bool is_model_different = selected_model && m_current_model ? selected_model->get_object_name() != m_current_model->get_object_name() : selected_model != m_current_model;
        if (is_model_different)
        {
            m_current_model = selected_model;
            //Scene::instance->set_model(m_current_model);
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
    }*/
}

void Details::draw_particle_system_section(ParticleSystem* particle_system)
{
    const int emitters_num = particle_system->get_emitters().size();
    ImGui::Text("Emitters (%d)", emitters_num);

    ImGui::SameLine();

    if (ImGui::Button("+", ImVec2(40, 0)))
    {
        particle_system->add_emitter();
    }

    ImGui::SameLine();

    if (ImGui::Button("-", ImVec2(40, 0)))
    {
        particle_system->remove_last_emitter();
    }

    for (auto& emitter : particle_system->get_emitters())
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
            ImGui::BeginDisabled(!emitter_config.randomize_scale);
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
            ImGuiB3D::ColorPicker4("Color", &emitter_config.color, "Control particle color in normalized RGBA channel.");
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