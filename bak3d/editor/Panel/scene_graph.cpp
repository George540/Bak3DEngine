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

#include "scene_graph.h"

#include "imgui_b3d_extensions.h"
#include "Scene/scene.h"
#include "Scene/scene_manager.h"

namespace
{
    void draw_subtree(const SceneObject* scene_object)
    {
        for (auto& child : scene_object->children)
        {
            if (!child || child->get_object_type() == SceneObjectType::Debug)
            {
                continue;
            }

            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
            if (SceneManager::get_current_scene()->get_selected_scene_object() == child.get())
            {
                flags |= ImGuiTreeNodeFlags_Selected;
            }

            if (child->children.empty())
            {
                flags |= ImGuiTreeNodeFlags_Leaf;
            }

            const bool is_grayed_out = !scene_object->is_active || !child->is_active;
            int pushed_colors = 0;
            if (is_grayed_out)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
                pushed_colors++;
            }

            const char* child_name = child->get_object_name().c_str();
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            const bool is_tree_open = ImGui::TreeNodeEx(child_name, flags, "%s", child_name);
            if (ImGui::IsItemClicked())
            {
                SceneManager::get_current_scene()->set_selected_scene_object(child.get());
            }

            if (is_tree_open)
            {
                draw_subtree(child.get());
                ImGui::TreePop();
            }

            if (pushed_colors > 0)
            {
                ImGui::PopStyleColor(pushed_colors);
            }
        }
    }
}

SceneGraph::SceneGraph() : EditorPanel("Scene")
{

}

void SceneGraph::begin_frame()
{
    EditorPanel::begin_frame();
}

void SceneGraph::update()
{
    EditorPanel::update();

    ImGuiB3D::SeparatorWithSpacing();

    draw_scene_graph();
}

void SceneGraph::end_frame()
{
    EditorPanel::end_frame();
}

void SceneGraph::draw_toolbar()
{
    draw_add_object_popup();
    ImGui::SameLine();
    ImGui::TextUnformatted("Add Object");
}

void SceneGraph::draw_scene_graph()
{
    if (const SceneObject* scene_root = SceneManager::get_current_scene()->get_root())
    {
        draw_subtree(scene_root);
    }

    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        if (!ImGui::IsAnyItemHovered())
        {
            SceneManager::get_current_scene()->set_selected_scene_object(nullptr);
        }
    }
}

void SceneGraph::draw_add_object_popup()
{
    if (ImGuiB3D::ColoredButton("+",  ImVec2(40, 0), ImVec4(0.0f, 0.5f, 0.0f, 1.0f)))
    {
        ImGui::OpenPopup("Add Object Popup");
    }

    if (ImGui::BeginPopup("Add Object Popup"))
    {
        ImGui::SeparatorText("Add Object");

        glm::vec3 spawn_position = glm::vec3(0.0f);
        if (const Camera* current_camera = SceneManager::get_current_scene()->get_current_camera())
        {
            spawn_position = current_camera->get_camera_position() + (current_camera->get_forward_vector() * 10.0f);
        }

        SceneObject* object_to_parent = SceneManager::get_current_scene()->get_selected_scene_object();

        if (ImGui::BeginMenu("Lights"))
        {
            if (ImGui::MenuItem("Directional Light"))
            {
                SceneManager::get_current_scene()->instantiate<Light>(object_to_parent, LightType::Directional, spawn_position);
            }
            if (ImGui::MenuItem("Spot Light"))
            {
                SceneManager::get_current_scene()->instantiate<Light>(object_to_parent, LightType::Spot, spawn_position);
            }
            if (ImGui::MenuItem("Point Light"))
            {
                SceneManager::get_current_scene()->instantiate<Light>(object_to_parent, LightType::Point, spawn_position);
            }
            if (ImGui::MenuItem("Area Light"))
            {
                // @TODO: Implement Area Light
                SceneManager::get_current_scene()->instantiate<Light>(object_to_parent, LightType::Point, spawn_position);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Primitives"))
        {
            ImGui::MenuItem("Cube");
            ImGui::MenuItem("Sphere");
            ImGui::MenuItem("Pyramid");
            ImGui::MenuItem("Cone");
            ImGui::MenuItem("Torus");
            ImGui::MenuItem("Suzanne");
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("VFX"))
        {
            if (ImGui::MenuItem("Sprite Particles"))
            {
                SceneManager::get_current_scene()->instantiate<ParticleSystem>(object_to_parent, spawn_position);
            }
            if (ImGui::MenuItem("Advanced Particles"))
            {
                SceneManager::get_current_scene()->instantiate<AdvancedParticleSystem>(object_to_parent, spawn_position);
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
}
