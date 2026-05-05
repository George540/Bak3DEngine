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

#include "asset_viewer.h"

#include <algorithm>
#include <utility>

#include "imgui_b3d_extensions.h"
#include "Asset/model.h"
#include "Asset/resource_manager.h"
#include "Asset/texture.h"

using namespace std;

namespace
{
    string m_search_name_string;
    string m_selected_texture;

    // Panel Settings
    float m_tile_size = 90.0f;
    float m_tile_padding = 8.0f;
    bool m_show_labels = true;
    bool m_show_tooltips = true;

    string truncate_label(const string& label, const float tile_width)
    {
        const float char_width = ImGui::CalcTextSize("A").x;
        int max_chars = static_cast<int>(tile_width / char_width);
        max_chars = max(max_chars, 1);

        if (cmp_less_equal(label.size(), max_chars))
        {
            return label;
        }

        return label.substr(0, max_chars - 3) + "...";
    }
}

AssetPanel::AssetPanel() : EditorPanel("Assets")
{
    
}

void AssetPanel::begin_frame()
{
    EditorPanel::begin_frame();
}

void AssetPanel::update()
{
    EditorPanel::update();

    draw_asset_toolbar();

    ImGuiB3D::SeparatorWithSpacing(1);

    draw_asset_grid();
}

void AssetPanel::end_frame()
{
    EditorPanel::end_frame();
}

void AssetPanel::draw_asset_toolbar()
{
    if (ImGui::Button("Clear", ImVec2(50, 20)))
    {
        
    }

    ImGui::SameLine();
    
    static char search_buffer[64] = "";
    ImGui::SetNextItemWidth(-1.0f);
    if (ImGui::InputTextWithHint("##asset_filter", "Filter assets by name...", search_buffer, IM_ARRAYSIZE(search_buffer)))
    {
        m_search_name_string = string(search_buffer);
    }
}

void AssetPanel::draw_asset_grid()
{
    const float cell_size = m_tile_size + m_tile_padding;
    const float panel_width = ImGui::GetContentRegionAvail().x;

    int m_columns = max(1, static_cast<int>(panel_width / cell_size));

    ImGui::BeginChild("##asset_grid", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    if (ImGui::BeginTable("##asset_table", m_columns, 0))
    {
        for (auto& [name, model] : ResourceManager::Models.all())
        {
            if (!m_search_name_string.empty() && !ImGuiB3D::StringContainsIgnoreCase(name, m_search_name_string))
            {
                continue;
            }

            ImGui::TableNextColumn();
            draw_asset_tile(name, model.ref()->asset);
        }
        for (auto& [name, texture] : ResourceManager::Textures.all())
        {
            if (!m_search_name_string.empty() && !ImGuiB3D::StringContainsIgnoreCase(name, m_search_name_string))
            {
                continue;
            }

            ImGui::TableNextColumn();
            draw_asset_tile(name, texture.ref()->asset);
        }

        ImGui::EndTable();
    }

    ImGui::EndChild();
}

void AssetPanel::draw_asset_tile(const string& name, Asset* asset)
{
    if (!asset)
    {
        return;
    }

    const bool is_selected = (m_selected_texture == name);
    const ImVec2 image_size = { m_tile_size, m_tile_size };
    const ImVec4 tint = is_selected
                        ? ImVec4(0.6f, 0.85f, 1.0f, 1.0f)  // highlight tint
                        : ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    const auto texture_asset = dynamic_cast<Texture2D*>(asset);
    const ImTextureID text_id = texture_asset ? texture_asset->get_texture_id() : asset->get_object_id();

    ImGui::PushID(name.c_str());

    // Selection highlight
    if (is_selected)
    {
        const ImVec2 cursor = ImGui::GetCursorScreenPos();
        ImGui::GetWindowDrawList()->AddRectFilled(
            cursor,
            { cursor.x + m_tile_size + 4.0f, cursor.y + m_tile_size + 4.0f },
            IM_COL32(100, 160, 255, 80),
            4.0f
        );
    }

    string label_asset = "##" + name;
    ImGui::BeginChild(label_asset.c_str(), ImVec2(image_size.x * 2, image_size.y * 1.3f));
    {
        // Image button
        const bool clicked = ImGui::ImageButton(
            label_asset.c_str(),
            text_id,
            image_size,
            { 1, 1}, { 0, 0 }, // UV coords (flip second pair for OpenGL if needed)
            ImVec4(0, 0, 0, 0), // background colour
            tint
        );

        if (clicked)
        {
            m_selected_texture = (is_selected ? "" : name); // toggle
        }

        // Label
        if (m_show_labels)
        {
            const string label = truncate_label(name, m_tile_size);
            const float text_width = ImGui::CalcTextSize(label.c_str()).x;
            const float indent = (m_tile_size - text_width) * 0.5f;
            if (indent > 0.0f)
            {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + indent);
            }
            ImGui::TextUnformatted(label.c_str());
        }
    }
    ImGui::EndChild();

    // Tooltip
    if (m_show_tooltips && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
    {
        ImGui::BeginTooltip();
        ImGui::Image(text_id, { 128.0f, 128.0f }, { 1, 1}, { 0, 0 });
        ImGui::Separator();
        ImGui::Text("File:       %s", asset->get_file_name().c_str());
        ImGui::Text("Directory:  %s", asset->get_directory().c_str());
        if (const Texture2D* texture = dynamic_cast<Texture2D*>(asset))
        {
            ImGui::Text("Resolution: %d x %d", texture->get_width(), texture->get_height());
            ImGui::Text("Channels:   %d", texture->get_nb_color_channels());
        }
        else if (const Model* model = dynamic_cast<Model*>(asset))
        {
            ImGui::Text("Vertices:   %u", model->get_vertices());
            ImGui::Text("Edges:      %u", model->get_unique_edges());
            ImGui::Text("Faces:      %u", model->get_faces());
        }
        ImGui::Text("GL ID:      %u", asset->get_object_id());
        ImGui::EndTooltip();
    }

    ImGui::PopID();
}
