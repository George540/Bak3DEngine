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

#include <ranges>

#include "imgui_b3d_extensions.h"
#include "Asset/mesh_data.h"
#include "Scene/scene.h"
#include "Scene/scene_manager.h"

namespace
{
    SceneObject* m_selected_object = nullptr;

    void draw_subtree(const SceneObject* scene_object)
    {
        for (auto& child : scene_object->children)
        {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
            if (m_selected_object == child.get())
            {
                flags |= ImGuiTreeNodeFlags_Selected;
            }

            if (child->children.empty())
            {
                flags |= ImGuiTreeNodeFlags_Leaf;
            }

            const char* child_name = child->get_object_name().c_str();
            const bool is_tree_open = ImGui::TreeNodeEx(child_name, flags, "%s", child_name);
            if (ImGui::IsItemClicked())
            {
                m_selected_object = child.get();
            }

            if (is_tree_open)
            {
                draw_subtree(child.get());
                ImGui::TreePop();
            }
        }
    }

    void set_object_text_visibility(RenderableObject* obj)
    {
        if (!obj)
        {
            return;
        }

        ImGui::TableNextRow(ImGuiTableRowFlags_None, 25.0f);
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();

        if (obj->is_visible())
        {
            ImGui::Text("%s", obj->get_object_name().c_str());
        }
        else
        {
            ImGui::TextDisabled("%s", obj->get_object_name().c_str());
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

    draw_toolbar();

    ImGuiB3D::SeparatorWithSpacing();

    draw_scene_graph();
}

void SceneGraph::end_frame()
{
    EditorPanel::end_frame();
}

void SceneGraph::draw_toolbar()
{
    ImGuiB3D::ColoredButton("+",  ImVec2(40, 0), ImVec4(0.0f, 0.5f, 0.0f, 1.0f));
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
            m_selected_object = nullptr;
        }
    }
}
