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
#include <ranges>
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

    void draw_truncated_tile_label(const string& asset_name)
    {
        const string label = ImGuiB3D::TruncateLabel(asset_name, m_tile_size);
        const float text_width = ImGui::CalcTextSize(label.c_str()).x;
        if (const float indent = (m_tile_size - text_width) * 0.5f; indent > 0.0f)
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + indent);
        }
        ImGui::TextUnformatted(label.c_str());
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

    build_asset_tree();

    draw_asset_toolbar();

    ImGuiB3D::SeparatorWithSpacing(1);

    draw_folder_tree();

    ImGui::SameLine();

    ImGui::BeginChild("##assets_panel", ImVec2(0, 0), true);
    draw_asset_grid(m_selected_folder ? *m_selected_folder : m_root);
    ImGui::EndChild();
}

void AssetPanel::end_frame()
{
    EditorPanel::end_frame();
}

void AssetPanel::draw_asset_toolbar()
{
    if (ImGui::Button("Clear", ImVec2(50, 20)))
    {
        m_search_name_string = "";
    }

    ImGui::SameLine();
    
    static char search_buffer[64] = "";
    ImGui::SetNextItemWidth(-1.0f);
    if (ImGui::InputTextWithHint("##asset_filter", "Filter assets by name...", search_buffer, IM_ARRAYSIZE(search_buffer)))
    {
        m_search_name_string = string(search_buffer);
    }
}

void AssetPanel::draw_folder_tree()
{
    ImGui::BeginChild("##folder_panel", ImVec2(180, 0), true);
    ImGui::SetNextItemOpen(true, ImGuiCond_Always);
    ImGuiTreeNodeFlags root_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (m_selected_folder == &m_root)
    {
        root_flags |= ImGuiTreeNodeFlags_Selected;
    }

    // Render the root node once using m_root data
    if (ImGui::TreeNodeEx(m_root.full_path.c_str(), root_flags, "%s", m_root.name.c_str()))
    {
        if (ImGui::IsItemClicked())
        {
            m_selected_folder = &m_root;
        }

        // Kick off the recursion for the child nodes
        draw_folder_subtree(m_root);

        ImGui::TreePop();
    }
    ImGui::EndChild();
}

void AssetPanel::draw_folder_subtree(AssetTreeNode& node)
{
    for (auto& child : node.sub_folders | views::values)
    {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (m_selected_folder == child.get())
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        const bool is_tree_open = ImGui::TreeNodeEx(child->full_path.c_str(), flags, "%s", child->name.c_str());
        if (ImGui::IsItemClicked())
        {
            m_selected_folder = child.get();
        }

        if (is_tree_open)
        {
            draw_folder_subtree(*child);
            ImGui::TreePop();
        }
    }
}

void AssetPanel::draw_asset_grid(AssetTreeNode& node)
{
    const float cell_size = m_tile_size + m_tile_padding;
    const int columns = max(1, static_cast<int>(ImGui::GetContentRegionAvail().x / cell_size));

    ImGui::BeginChild("##asset_grid", ImVec2(0, 0.0f), false, ImGuiWindowFlags_HorizontalScrollbar);
    if (ImGui::BeginTable("##asset_table", columns, 0))
    {
        for (auto& [name, child] : node.sub_folders)
        {
            if (!m_search_name_string.empty() && !ImGuiB3D::StringContainsIgnoreCase(name, m_search_name_string))
            {
                continue;
            }
            ImGui::TableNextColumn();
            draw_folder_tile(child.get());
        }

        for (auto& [name, asset] : node.assets)
        {
            if (!m_search_name_string.empty() && !ImGuiB3D::StringContainsIgnoreCase(name, m_search_name_string))
            {
                continue;
            }
            ImGui::TableNextColumn();
            draw_asset_tile(name, asset);
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();
}

void AssetPanel::draw_folder_tile(AssetTreeNode* folder)
{
    ImGui::PushID(folder);

    const ImVec2 image_size = { m_tile_size, m_tile_size };
    const string label_asset = "##folder_" + folder->full_path;

    ImGui::BeginChild(label_asset.c_str(), ImVec2(image_size.x * 2, image_size.y * 1.3f));
    {
        const TextureRef folder_icon = !folder->sub_folders.empty() || !folder->assets.empty() ? ResourceManager::get_texture("folder_icon.png") : ResourceManager::get_texture("folder_icon_empty.png");
        const ImTextureID texture_id = folder_icon ? folder_icon->get_texture_id() : 0;

        const bool is_folder_clicked = ImGui::ImageButton(
            label_asset.c_str(),
            texture_id,
            image_size,
            { 0, 1 }, { 1, 0 },
            ImVec4(0, 0, 0, 0),
            ImVec4(1, 1, 1, 1)
        );

        if (is_folder_clicked)
        {
            m_selected_folder = folder;
        }

        // Label
        if (m_show_labels)
        {
            draw_truncated_tile_label(folder->name);
        }
    }
    ImGui::EndChild();
    
    ImGui::PopID();
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
            { 0, 1 }, { 1, 0 }, // UV coords (flip second pair for OpenGL if needed)
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
            draw_truncated_tile_label(name);
        }
    }
    ImGui::EndChild();

    // Tooltip
    if (m_show_tooltips && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
    {
        ImGuiB3D::AssetTooltip(asset);
    }

    ImGui::PopID();
}

template <typename T>
void AssetPanel::insert_to_tree(const ResourceMap<T>& map)
{
    for (auto& [name, ref] : map.all())
    {
        Asset* asset = ref.ref()->asset;
        if (!asset)
        {
            continue;
        }

        // Skip texture assets that are meant to be for editor-based entities only.
        // Those are not viewable or usable in the engine.
        if (const auto* texture = dynamic_cast<Texture2D*>(asset); texture && texture->is_editor_texture())
        {
            continue;
        }

        auto base_directory = string(BAK3D_ASSETS_DIR);
        string directory = asset->get_directory();
        if (directory.contains(base_directory))
        {
            directory = directory.substr(base_directory.size());
        }
        
        // Remove trailing slashes, if any
        while (!directory.empty() && (directory.front() == '/' || directory.front() == '\\'))
        {
            directory.erase(directory.begin());
        }

        AssetTreeNode* current = &m_root;
        size_t start = 0;
        while (start < directory.size())
        {
            const size_t slash = directory.find_first_of('/', start);
            if (string segment = directory.substr(start, slash == string::npos ? string::npos : slash - start); !segment.empty())
            {
                auto it = current->sub_folders.find(segment);
                if (it == current->sub_folders.end())
                {
                    auto node = make_unique<AssetTreeNode>();
                    node->name = segment;
                    node->full_path = current->full_path.empty() ? segment : current->full_path + "/" + segment;
                    it = current->sub_folders.emplace(segment, move(node)).first;
                }
                current = it->second.get();
            }

            if (slash == string::npos)
            {
                break;
            }
            start = slash + 1;
        }

        current->assets.emplace_back(name, asset);
    }
}

void AssetPanel::build_asset_tree()
{
    const size_t current_assets_count = ResourceManager::Models.size() + ResourceManager::Textures.size();
    if (current_assets_count == m_last_asset_count)
    {
        return;
    }

    string previous_path = m_selected_folder ? m_selected_folder->full_path : "";

    m_root = AssetTreeNode{};
    m_root.full_path = string(BAK3D_ASSETS_DIR);
    const size_t root_folder_index = m_root.full_path.find_last_of('/');
    m_root.name = m_root.full_path.substr(root_folder_index + 1);
    insert_to_tree(ResourceManager::Models);
    insert_to_tree(ResourceManager::Textures);
    m_last_asset_count = current_assets_count;
}
