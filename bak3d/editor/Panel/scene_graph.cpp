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

#include "Asset/mesh_data.h"
#include "Scene/scene.h"

namespace
{
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

    /*ImGui::BeginTable("##scene_graph", 1, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_ScrollY);
    {
        Light* light_object = dynamic_cast<Light*>(Scene::instance->get_object_in_scene(SceneObjectType::Light));
        Model* model_object = Scene::instance->get_model();

        set_object_text_visibility(grid_object);
        set_object_text_visibility(axis_object);
        set_object_text_visibility(light_object);

        if (model_object)
        {
            ImGui::TableNextRow(ImGuiTableRowFlags_None, 25.0f);
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGui::Text("%s", model_object->get_object_name().c_str());

            for (const auto& child_mesh : model_object->get_all_mesh_data())
            {
                ImGui::TableNextRow(ImGuiTableRowFlags_None, 25.0f);
                ImGui::TableSetColumnIndex(0);
                ImGui::AlignTextToFramePadding();
                ImGui::Text("... %s", child_mesh->get_object_name().c_str());
            }
        }
    }
    ImGui::EndTable();*/
}

void SceneGraph::end_frame()
{
    EditorPanel::end_frame();
}
