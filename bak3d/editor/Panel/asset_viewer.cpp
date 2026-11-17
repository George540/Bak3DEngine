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
#include "Scene/scene_manager.h"

using namespace std;

namespace
{
    string m_search_name_string;
    string m_selected_texture;

    float m_tile_size = 90.0f;
    bool m_show_labels = true;
    bool m_show_tooltips = true;

    constexpr float TILE_INNER_PADDING = 6.0f;   // space between button edge and image/label
    constexpr float TILE_LABEL_GAP = 4.0f;       // image to label
    constexpr ImVec2 TILE_CELL_PADDING = ImVec2(3.0f, 3.0f); // gap between tiles

    float tile_width()
    {
        return m_tile_size + TILE_INNER_PADDING * 2.0f;
    }

    float tile_height()
    {
        float height = m_tile_size + TILE_INNER_PADDING * 2.0f;
        if (m_show_labels)
        {
            height += TILE_LABEL_GAP + ImGui::GetTextLineHeight();
        }
        return height;
    }

    // Button background + image + truncated label drawn inside the button's rect
    bool draw_tile_button(const string& id, const ImTextureID texture_id, const string& label)
    {
        const ImVec2 size(tile_width(), tile_height());
        const bool clicked = ImGui::Button(id.c_str(), size);

        const ImVec2 rect_min = ImGui::GetItemRectMin();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        const ImVec2 image_min(rect_min.x + TILE_INNER_PADDING, rect_min.y + TILE_INNER_PADDING);
        const ImVec2 image_max(image_min.x + m_tile_size, image_min.y + m_tile_size);
        draw_list->AddImage(texture_id, image_min, image_max, ImVec2(0, 1), ImVec2(1, 0));

        if (m_show_labels)
        {
            const string text = ImGuiB3D::TruncateLabel(label, m_tile_size);
            const float text_width = ImGui::CalcTextSize(text.c_str()).x;
            const ImVec2 text_pos(rect_min.x + (size.x - text_width) * 0.5f, image_max.y + TILE_LABEL_GAP);
            draw_list->AddText(text_pos, ImGui::GetColorU32(ImGuiCol_Text), text.c_str());
        }

        return clicked;
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
    build_asset_tree();

    EditorPanel::update();

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

void AssetPanel::draw_toolbar()
{
    ImGui::SetNextItemWidth(120.0f);
    ImGui::SliderFloat("##tile_size", &m_tile_size, 30.0f, 160.0f, "%.0f px");
    ImGuiB3D::ToolTipExtendedText("Controls the size of the asset and folder tiles in the grid, in pixels.");

    ImGui::SameLine();

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
    const float cell_size = tile_width() + TILE_CELL_PADDING.x * 2.0f;
    const float available_width = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ScrollbarSize;
    const int columns = max(1, static_cast<int>(available_width / cell_size));

    ImGui::BeginChild("##asset_grid", ImVec2(0, 0.0f), false, ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, TILE_CELL_PADDING);
    if (ImGui::BeginTable("##asset_table", columns, ImGuiTableFlags_SizingFixedFit))
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
    ImGui::PopStyleVar();
    ImGui::EndChild();
}

void AssetPanel::draw_folder_tile(AssetTreeNode* folder)
{
    ImGui::PushID(folder);

    const TextureRef folder_icon = !folder->sub_folders.empty() || !folder->assets.empty() ? ResourceManager::get_texture("folder_icon.png") : ResourceManager::get_texture("folder_icon_empty.png");
    const ImTextureID texture_id = folder_icon ? folder_icon->get_texture_id() : 0;

    if (draw_tile_button("##folder_button", texture_id, folder->name))
    {
        m_selected_folder = folder;
    }

    ImGui::PopID();
}

void AssetPanel::draw_asset_tile(const string& name, Asset* asset)
{
    if (!asset)
    {
        return;
    }

    const ImVec2 image_size = { m_tile_size, m_tile_size };

    const auto texture_asset = dynamic_cast<Texture2D*>(asset);
    const GLuint thumbnail_id = asset->get_thumbnail_id();
    ImTextureID texture_id;
    if (texture_asset)
    {
        texture_id = texture_asset->get_texture_id();
    }
    else if (thumbnail_id != 0)
    {
        texture_id = thumbnail_id;
    }
    else
    {
        texture_id = asset->get_object_id();
    }

    ImGui::PushID(name.c_str());

    const bool clicked = draw_tile_button("##asset_button", texture_id, name);

    if (ImGui::IsItemActive() || ImGui::IsItemHovered())
    {
        const ImU32 overlay_color = ImGui::IsItemActive()
            ? IM_COL32(100, 160, 255, 120)
            : IM_COL32(100, 160, 255, 60);
        ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), overlay_color, 4.0f);
    }

    if (m_show_tooltips && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
    {
        ImGuiB3D::AssetTooltip(asset);
    }

    if (const auto model_asset = dynamic_cast<Model*>(asset); clicked && model_asset)
    {
        const glm::vec3 spawn_position = SceneManager::get_current_scene()->process_spawn_position();
        SceneObject* object_to_parent = SceneManager::get_current_scene()->get_selected_scene_object();
        SceneManager::get_current_scene()->instantiate_model(ResourceManager::get_model(model_asset->get_file_name()), object_to_parent, spawn_position);
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
    const size_t current_assets_count = ResourceManager::Models.size()
                                        + ResourceManager::Textures.size()
                                        + ResourceManager::Materials.size();
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
    insert_to_tree(ResourceManager::Materials);
    m_last_asset_count = current_assets_count;
}
