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

#include "imgui_b3d_extensions.h"

#include <algorithm>
#include <imgui.h>
#include <iterator>
#include <string>

using namespace std;

constexpr float TOOL_TIP_WIDTH = 500.0f;
constexpr float LABEL_HORIZONTAL_WIDTH_RATIO = 0.5f;
constexpr float VALUE_HORIZONTAL_WIDTH_RATIO = 0.1f;
constexpr float VALUE_INNER_PADDING = 5.0f;

bool ImGuiB3D::PropertyToggle(const char* label, bool* value, const char* tooltip_desc)
{
    ImGui::TextUnformatted(label);
    if (tooltip_desc && ImGui::IsItemHovered())
    {
        ToolTipExtendedText(tooltip_desc, TOOL_TIP_WIDTH);
    }
    ImGui::SameLine(ImGui::GetContentRegionAvail().x * LABEL_HORIZONTAL_WIDTH_RATIO);
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * (1.0f - VALUE_HORIZONTAL_WIDTH_RATIO) - VALUE_INNER_PADDING);
    const auto label_str = "##" + string(label);
    return ImGui::Checkbox(label_str.c_str(), value);
}

bool ImGuiB3D::PropertyColorPicker(const char* label, float* color, const char* tooltip_desc)
{
    ImGui::TextUnformatted(label);
    if (tooltip_desc && ImGui::IsItemHovered())
    {
        ToolTipExtendedText(tooltip_desc, TOOL_TIP_WIDTH);
    }
    ImGui::SameLine(ImGui::GetContentRegionAvail().x * LABEL_HORIZONTAL_WIDTH_RATIO);
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * (1.0f - VALUE_HORIZONTAL_WIDTH_RATIO) - VALUE_INNER_PADDING);
    const auto label_str = "##" + string(label);
    return ImGui::ColorEdit4(label_str.c_str(), color);
}

bool ImGuiB3D::PropertyDropdown(const char* label, const std::vector<const char*>& data, int* selected_index, const char* tooltip_desc)
{
    ImGui::TextUnformatted(label);
    if (tooltip_desc && ImGui::IsItemHovered())
    {
        ToolTipExtendedText(tooltip_desc, TOOL_TIP_WIDTH);
    }
    ImGui::SameLine(ImGui::GetContentRegionAvail().x * LABEL_HORIZONTAL_WIDTH_RATIO);
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * (1.0f - VALUE_HORIZONTAL_WIDTH_RATIO) - VALUE_INNER_PADDING);
    const auto label_str = "##" + string(label);
    return ImGui::Combo(label_str.c_str(), selected_index, data.data(), static_cast<int>(data.size()));
}

bool ImGuiB3D::PropertyBeginDropdown(const char* label, const char* preview_value, const char* tooltip_desc)
{
    ImGui::TextUnformatted(label);
    if (tooltip_desc && ImGui::IsItemHovered())
    {
        ToolTipExtendedText(tooltip_desc, TOOL_TIP_WIDTH);
    }
    ImGui::SameLine(ImGui::GetContentRegionAvail().x * LABEL_HORIZONTAL_WIDTH_RATIO);
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * (1.0f - VALUE_HORIZONTAL_WIDTH_RATIO) - VALUE_INNER_PADDING);
    const auto label_str = "##" + string(label);
    return ImGui::BeginCombo(label_str.c_str(), preview_value);
    // NOTE: Make sure you use ImGui::EndCombo() to properly close the widget.
}

bool ImGuiB3D::PropertySliderFloat(const char* label, float* value, float v_min, float v_max, const char* format,
    const char* tooltip_desc)
{
    ImGui::TextUnformatted(label);
    if (tooltip_desc && ImGui::IsItemHovered())
    {
        ToolTipExtendedText(tooltip_desc, TOOL_TIP_WIDTH);
    }
    ImGui::SameLine(ImGui::GetContentRegionAvail().x * LABEL_HORIZONTAL_WIDTH_RATIO);
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * (1.0f - VALUE_HORIZONTAL_WIDTH_RATIO) - VALUE_INNER_PADDING);
    const auto label_str = "##" + string(label);
    return ImGui::SliderFloat(label_str.c_str(), value, v_min, v_max, format);
}

bool ImGuiB3D::PropertyButton(const char* property_label, const char* button_label, const char* tooltip_desc, const ImVec2 size)
{
    ImGui::TextUnformatted(property_label);
    if (tooltip_desc && ImGui::IsItemHovered())
    {
        ToolTipExtendedText(tooltip_desc, TOOL_TIP_WIDTH);
    }
    ImGui::SameLine(ImGui::GetContentRegionAvail().x * LABEL_HORIZONTAL_WIDTH_RATIO);
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * (1.0f - VALUE_HORIZONTAL_WIDTH_RATIO) - VALUE_INNER_PADDING);

    const char* display_text = button_label ? button_label : property_label;
    char display_text_buffer[128];
    auto result = snprintf(display_text_buffer, sizeof(display_text_buffer), "%s##%s", display_text, property_label);
    return ImGui::Button(display_text_buffer, size);
}

bool ImGuiB3D::PropertyImageButton(const char* label, const char* tooltip_desc, const ImTextureID texture_id, const ImVec2 size,
                                   const ImVec2 uv0, const ImVec2 uv1, const ImVec4 bg_col, const ImVec4 tint_col)
{
    ImGui::TextUnformatted(label);
    if (tooltip_desc && ImGui::IsItemHovered())
    {
        ToolTipExtendedText(tooltip_desc, TOOL_TIP_WIDTH);
    }
    ImGui::SameLine(ImGui::GetContentRegionAvail().x * LABEL_HORIZONTAL_WIDTH_RATIO);
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * (1.0f - VALUE_HORIZONTAL_WIDTH_RATIO) - VALUE_INNER_PADDING);
    const auto label_str = "##" + string(label);
    return ImGui::ImageButton(
            label,
            texture_id,
            size,
            uv0, uv1, // (0,0) and (1,1), flip pairs if needed (most of the time)
            bg_col,
            tint_col);
}

bool ImGuiB3D::ToolTipExtendedText(const char* tooltip_desc, float text_wrap_size)
{
    ImGui::BeginTooltip();
    ImGui::PushTextWrapPos(text_wrap_size);
    ImGui::TextUnformatted(tooltip_desc);
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
    return true;
}

bool ImGuiB3D::MultiSpacing(int num_spaces)
{
    for (int i = 0; i < num_spaces; i++)
    {
        ImGui::Spacing();
    }
    return true;
}

bool ImGuiB3D::SeparatorWithSpacing(const int num_spaces = 1)
{
    MultiSpacing(num_spaces);
    ImGui::Separator();
    MultiSpacing(num_spaces);
    return true;
}

bool ImGuiB3D::StringContainsIgnoreCase(string str, const string& sub_str)
{
    ranges::transform(str, str.begin(), 
                          [](const unsigned char c){ return std::tolower(c); });

    string sub_lower;
    ranges::transform(sub_str, std::back_inserter(sub_lower),
                      [](const unsigned char c){ return std::tolower(c); });

    return str.find(sub_lower) != std::string::npos;
}
