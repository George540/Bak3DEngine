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

#include "Asset/model.h"
#include "Asset/texture.h"

using namespace std;

constexpr float TOOL_TIP_WIDTH = 500.0f;
constexpr float LABEL_HORIZONTAL_WIDTH_RATIO = 0.5f;
constexpr float VALUE_HORIZONTAL_WIDTH_RATIO = 0.1f;
constexpr float VALUE_INNER_PADDING = 5.0f;

namespace
{
    float align_to_label_column()
    {
        const float total_width  = ImGui::GetContentRegionAvail().x;  // snapshot before cursor moves
        const float label_col    = total_width * LABEL_HORIZONTAL_WIDTH_RATIO;
        const float widget_width = total_width - label_col - VALUE_INNER_PADDING;

        ImGui::SameLine(label_col);

        return widget_width;
    }
}

bool ImGuiB3D::PropertyToggle(const char* label, bool* value, const char* tooltip_desc)
{
    ImGui::TextUnformatted(label);
    if (tooltip_desc && ImGui::IsItemHovered())
    {
        ToolTipExtendedText(tooltip_desc, TOOL_TIP_WIDTH);
    }
    ImGui::SetNextItemWidth(align_to_label_column());
    const auto label_str = "##" + string(label);
    return ImGui::Checkbox(label_str.c_str(), value);
}

bool ImGuiB3D::PropertyToggle(const char* label, int* value, const char* tooltip_desc)
{
    ImGui::TextUnformatted(label);
    if (tooltip_desc && ImGui::IsItemHovered())
    {
        ToolTipExtendedText(tooltip_desc, TOOL_TIP_WIDTH);
    }
    ImGui::SetNextItemWidth(align_to_label_column());
    const auto label_str = "##" + string(label);
    bool bool_value = (*value != 0);
    const bool success = ImGui::Checkbox(label_str.c_str(), &bool_value);
    *value = static_cast<int>(bool_value);
    return success;
}

bool ImGuiB3D::PropertyColorPicker(const char* label, glm::vec4* color, const char* tooltip_desc)
{
    ImGui::TextUnformatted(label);
    if (tooltip_desc && ImGui::IsItemHovered())
    {
        ToolTipExtendedText(tooltip_desc, TOOL_TIP_WIDTH);
    }
    ImGui::SetNextItemWidth(align_to_label_column());
    const auto label_str = "##" + string(label);
    float color_channels[4] = { color->r, color->g, color->b, color->a };
    const bool result = ImGui::ColorEdit4(label_str.c_str(), color_channels);
    color->r = color_channels[0];
    color->g = color_channels[1];
    color->b = color_channels[2];
    color->a = color_channels[3];
    return result;
}

bool ImGuiB3D::PropertyDropdown(const char* label, const std::vector<const char*>& data, int* selected_index, const char* tooltip_desc)
{
    ImGui::TextUnformatted(label);
    if (tooltip_desc && ImGui::IsItemHovered())
    {
        ToolTipExtendedText(tooltip_desc, TOOL_TIP_WIDTH);
    }
    ImGui::SetNextItemWidth(align_to_label_column());
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
    ImGui::SetNextItemWidth(align_to_label_column());
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
    ImGui::SetNextItemWidth(align_to_label_column());
    const auto label_str = "##" + string(label);
    return ImGui::SliderFloat(label_str.c_str(), value, v_min, v_max, format);
}

bool ImGuiB3D::PropertySliderFloat3(const char* label, glm::vec3* value, float v_min, float v_max, const char* format,
    const char* tooltip_desc)
{
    ImGui::TextUnformatted(label);
    if (tooltip_desc && ImGui::IsItemHovered())
    {
        ToolTipExtendedText(tooltip_desc, TOOL_TIP_WIDTH);
    }
    ImGui::SetNextItemWidth(align_to_label_column());
    const auto label_str = "##" + string(label);
    float float_3[3] = {  value->r,  value->g,  value->b };
    const bool result = ImGui::SliderFloat3(label_str.c_str(), float_3, v_min, v_max, format);
    value->r = float_3[0];
    value->g = float_3[1];
    value->b = float_3[2];
    return result;
}

bool ImGuiB3D::PropertySliderFloat4(const char* label, glm::vec4* value, float v_min, float v_max, const char* format,
    const char* tooltip_desc)
{
    ImGui::TextUnformatted(label);
    if (tooltip_desc && ImGui::IsItemHovered())
    {
        ToolTipExtendedText(tooltip_desc, TOOL_TIP_WIDTH);
    }
    ImGui::SetNextItemWidth(align_to_label_column());
    const auto label_str = "##" + string(label);
    float float_4[4] = {  value->r,  value->g,  value->b, value->a };
    const bool result = ImGui::SliderFloat4(label_str.c_str(), float_4, v_min, v_max, format);
    value->r = float_4[0];
    value->g = float_4[1];
    value->b = float_4[2];
    value->a = float_4[4];
    return result;
}

bool ImGuiB3D::PropertySliderInt(const char* label, int* value, int v_min, int v_max, const char* tooltip_desc)
{
    ImGui::TextUnformatted(label);
    if (tooltip_desc && ImGui::IsItemHovered())
    {
        ToolTipExtendedText(tooltip_desc, TOOL_TIP_WIDTH);
    }
    ImGui::SetNextItemWidth(align_to_label_column());
    const auto label_str = "##" + string(label);
    return ImGui::SliderInt(label_str.c_str(), value, v_min, v_max);
}

bool ImGuiB3D::PropertyButton(const char* property_label, const char* button_label, const char* tooltip_desc, const ImVec2 size)
{
    ImGui::TextUnformatted(property_label);
    if (tooltip_desc && ImGui::IsItemHovered())
    {
        ToolTipExtendedText(tooltip_desc, TOOL_TIP_WIDTH);
    }
    align_to_label_column();

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
    align_to_label_column();
    const auto label_str = "##" + string(label);
    return ImGui::ImageButton(
            label,
            texture_id,
            size,
            uv0, uv1, // (0,0) and (1,1), flip pairs if needed (most of the time)
            bg_col,
            tint_col);
}

string ImGuiB3D::TruncateLabel(const string& label, const float tile_width)
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

void ImGuiB3D::AssetTooltip(const Asset* asset)
{
    if (!asset)
    {
        return;
    }

    const auto* texture = dynamic_cast<const Texture2D*>(asset);
    const ImTextureID tex_id = texture ? texture->get_texture_id() : asset->get_object_id();

    ImGui::BeginTooltip();
    ImGui::Image(tex_id, { 128.0f, 128.0f }, { 1, 1 }, { 0, 0 });
    ImGui::Separator();
    ImGui::Text("File:       %s", asset->get_file_name().c_str());
    ImGui::Text("Directory:  %s", asset->get_directory().c_str());
    if (texture)
    {
        ImGui::Text("Resolution: %d x %d", texture->get_width(), texture->get_height());
        ImGui::Text("Channels:   %d", texture->get_nb_color_channels());
        ImGui::Text("Texture ID: %d", texture->get_texture_id());
    }
    else if (const auto* model = dynamic_cast<const Model*>(asset))
    {
        ImGui::Text("Vertices:   %u", model->get_vertices());
        ImGui::Text("Edges:      %u", model->get_unique_edges());
        ImGui::Text("Faces:      %u", model->get_faces());
    }
    ImGui::Text("GL ID:      %u", asset->get_object_id());
    ImGui::EndTooltip();
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

bool ImGuiB3D::SeparatorWithSpacing(const int num_spaces)
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

bool ImGuiB3D::InteractableMultilineText(const std::string& label, const std::string& text, const ImVec4 color)
{
    // Push styling
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 0.0f);

    char log_buffer[512];
    strncpy_s(log_buffer, text.c_str(), sizeof(log_buffer) - 1);
    log_buffer[sizeof(log_buffer) - 1] = '\0';
    const string text_label = "##" + label;

    const float width = ImGui::GetContentRegionAvail().x;
    const ImVec2 text_size = ImGui::CalcTextSize(log_buffer, nullptr, false, width);
    const float min_height = ImGui::GetTextLineHeightWithSpacing();
    const float dynamic_height = std::max(text_size.y + ImGui::GetStyle().FramePadding.y * 2.0f, min_height);

    const bool result = ImGui::InputTextMultiline(text_label.c_str(), log_buffer, IM_ARRAYSIZE(log_buffer), ImVec2(width, dynamic_height), ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_WordWrap | ImGuiInputTextFlags_NoHorizontalScroll);

    // Pop Styling
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    return result;
}

bool ImGuiB3D::ColoredButton(const char* label, const ImVec2 size, const ImVec4 color)
{
    ImGui::PushStyleColor(ImGuiCol_Button, color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(color.x + BUTTON_HOVERED_LIGHTEN_BOOST, color.y + BUTTON_HOVERED_LIGHTEN_BOOST, color.z + BUTTON_HOVERED_LIGHTEN_BOOST, color.w));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(color.x + BUTTON_ACTIVE_LIGHTEN_BOOST, color.y + BUTTON_ACTIVE_LIGHTEN_BOOST, color.z + BUTTON_ACTIVE_LIGHTEN_BOOST, color.w));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

    const bool is_pressed = ImGui::Button(label, size);

    ImGui::PopStyleColor(4);

    return is_pressed;
}
