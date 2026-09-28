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

#pragma once

#include <imgui.h>
#include <string>
#include <vector>

#include "Asset/asset.h"
#include "Asset/resource_map.h"
#include "Asset/texture.h"

constexpr static float TOOL_TIP_WIDTH = 500.0f;
constexpr static float BUTTON_LIGHTEN_BOOST = 0.25f;
constexpr static float BUTTON_HOVERED_LIGHTEN_BOOST = 0.1f;
constexpr static float BUTTON_ACTIVE_LIGHTEN_BOOST = 0.2f;
constexpr static ImVec2 BUTTON_SIZE = ImVec2(60.0f, 20.0f);

class ImGuiB3D
{
public:
    static bool ToolTipExtendedText(const char* tooltip_desc, float text_wrap_size = TOOL_TIP_WIDTH);
    static bool PropertyToggle(const char* label, bool* value, const char* tooltip_desc = nullptr);
    static bool PropertyToggle(const char* label, int* value, const char* tooltip_desc = nullptr);
    static bool ColorPicker3(const char* label, glm::vec3* color, const char* tooltip_desc = nullptr, bool is_property = true, ImVec2 channel_size = ImVec2(0.0f, 0.0f));
    static bool ColorPicker4(const char* label, glm::vec4* color, const char* tooltip_desc = nullptr, bool is_property = true, ImVec2 channel_size = ImVec2(0.0f, 0.0f));
    static bool PropertyDropdown(const char* label, const std::vector<const char*>& data, int* selected_index, const char* tooltip_desc = nullptr);
    static bool PropertyBeginDropdown(const char* label, const char* preview_value, const char* tooltip_desc);
    static bool PropertyDragFloat(const char* label, float* value, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", const char* tooltip_desc = nullptr);
    static bool PropertyDragFloat3(const char* label, glm::vec3* value, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", const char* tooltip_desc = nullptr, const bool is_colored = true);
    static bool PropertySliderFloat(const char* label, float* value, float v_min, float v_max, const char* format, const char* tooltip_desc = nullptr);
    static bool PropertySliderFloat3(const char* label, glm::vec3* value, float v_min, float v_max, const char* format, const char* tooltip_desc = nullptr);
    static bool PropertySliderFloat4(const char* label, glm::vec4* value, float v_min, float v_max, const char* format, const char* tooltip_desc = nullptr);
    static bool PropertySliderInt(const char* label, int* value, int v_min, int v_max, const char* tooltip_desc = nullptr);
    static bool PropertyButton(const char* property_label, const char* button_label = nullptr, const char* tooltip_desc = nullptr, ImVec2 size = ImVec2(0, 0));
    static bool PropertyImageButton(const char* label, const char* tooltip_desc, ImTextureID texture_id, ImVec2 size, ImVec2 uv0 = ImVec2( 1.0f, 1.0f), ImVec2 uv1 = ImVec2( 0.0f, 0.0f), ImVec4 bg_col = ImVec4(0.0f, 0.0f, 0.0f, 0.0f), ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    
    static std::string TruncateLabel(const std::string& label, float tile_width);
    static void AssetTooltip(const Asset* asset);

    static bool SeparatorWithSpacing(int num_spaces = 1);
    static bool StringContainsIgnoreCase(std::string str, const std::string& sub_str);
    static bool InteractableMultilineText(const std::string& label, const std::string& text, ImVec4 color);
    static bool ColoredButton(const char* label, ImVec2 size = ImVec2(0, 0), ImVec4 color = ImVec4(1, 1, 1, 1));

    static bool MultiSpacing(int num_spaces);

    static ImTextureID GetAssetPreviewTextureId(const Asset* asset);
    /*
     * Generic asset picker popup.
     *
     * popup_id       - unique ImGui ID string for this popup (e.g. "##sprite_picker")
     * separator_text - label shown above the search bar
     * asset_map      - any ResourceMap<T>; iterated to build the grid
     * selected_name  - string address of selected asset name
     * filter         - optional filter algorithm for filtering specific types of assets of the same type (ex: Filtering GridData from a pool of PrimitiveData)
     * tile_size      - icon size in pixels (default 56)
     *
     * Returns true the frame a selection is made.
     *
     * Usage:
     *   if (ImGui::Button("Pick texture")) ImGui::OpenPopup("##my_picker");
     *   ImGuiB3D::AssetPickerPopup(
     *       "##my_picker", "Textures",
     *       ResourceManager::Textures
     *   );
     */
    template<typename T>
    static bool AssetPickerPopup(
        const char*           popup_id,
        const char*           separator_text,
        const ResourceMap<T>& asset_map,
        std::string*          selected_name,
        const std::function<bool(const Asset*)>& filter = nullptr,
        const float           tile_size = 50.0f)
    {
        static std::unordered_map<std::string, std::string> s_search_buffers;
        std::string& search = s_search_buffers[popup_id];

        const bool is_pop_up_open = ImGui::IsPopupOpen(popup_id);

        if (is_pop_up_open)
        {
            ImGui::SetNextWindowSizeConstraints(ImVec2(200, 300), ImVec2(400, 400));
        }

        if (ImGui::BeginPopup(popup_id, ImGuiWindowFlags_NoMove))
        {
            ImGui::SeparatorText(separator_text);

            char search_buf[64] = {};
            const size_t copy_len = std::min(search.size(), sizeof(search_buf) - 1);
            std::memcpy(search_buf, search.c_str(), copy_len);

            ImGui::SetNextItemWidth(-1.0f);
            if (ImGui::InputTextWithHint("##asset_picker_search", "Search...", search_buf, IM_ARRAYSIZE(search_buf)))
            {
                search = search_buf;
            }

            ImGui::Spacing();

            const ImVec2 availableSpace = ImGui::GetContentRegionAvail();
            const std::string scrollable_region_id = std::string("##ScrollableRegion_") + popup_id;
            if (ImGui::BeginChild(scrollable_region_id.c_str(), availableSpace, ImGuiChildFlags_None, ImGuiWindowFlags_None))
            {
                for (auto& [name, asset_ref] : asset_map.all())
                {
                    // Skip if asset is not in the search
                    if (!search.empty() && !StringContainsIgnoreCase(name, search))
                    {
                        continue;
                    }

                    // Skip if asset is null
                    const Asset* asset = asset_ref.ref()->asset;
                    if (!asset)
                    {
                        continue;
                    }

                    // Skip if asset is an editor-related texture that is unusable
                    if (const auto* texture = dynamic_cast<const Texture2D*>(asset); texture && texture->is_editor_texture())
                    {
                        continue;
                    }

                    // Skip if asset is not in the filter algorithm
                    if (filter && !filter(asset))
                    {
                        continue;
                    }

                    const ImVec2 icon_size = ImVec2(tile_size, tile_size);
                    if (ImGui::ImageButton(name.c_str(), GetAssetPreviewTextureId(asset), icon_size, { 0, 1 }, { 1, 0 }))
                    {
                        *selected_name = name; // resource map key, not file name
                        ImGui::CloseCurrentPopup();
                    }

                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone))
                    {
                        AssetTooltip(asset);
                    }

                    ImGui::SameLine();

                    const std::string trunc = TruncateLabel(name, ImGui::GetContentRegionAvail().x);
                    ImGui::TextUnformatted(trunc.c_str());
                }

                ImGui::EndChild();
            }

            ImGui::EndPopup();
        }

        return is_pop_up_open;
    }
};
