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

#include "console.h"

#include <imgui_internal.h>
#include "imgui_b3d_extensions.h"
#include "Core/logger.h"

using namespace std;

namespace
{
    int log_filter_mask =
        static_cast<int>(LogLevel::Log_Info) |
        static_cast<int>(LogLevel::Log_Warning) |
        static_cast<int>(LogLevel::Log_Error);

    constexpr float BUTTON_LIGHTEN_BOOST = 0.25f;
    constexpr float BUTTON_HOVERED_LIGHTEN_BOOST = 0.1f;
    constexpr float BUTTON_ACTIVE_LIGHTEN_BOOST = 0.2f;
    constexpr ImVec2 BUTTON_SIZE = ImVec2(60.0f, 20.0f);

    void draw_log_filter_button(const char* label, LogLevel log_level, const ImVec2& size, const ImVec4& base_color)
    {
        const bool enabled = (log_filter_mask & static_cast<int>(log_level)) != 0;

        // Lighten color if active
        ImVec4 color = base_color;
        if (enabled)
        {
            color.x = ImMin(color.x + BUTTON_LIGHTEN_BOOST, 1.0f);
            color.y = ImMin(color.y + BUTTON_LIGHTEN_BOOST, 1.0f);
            color.z = ImMin(color.z + BUTTON_LIGHTEN_BOOST, 1.0f);
        }

        // Apply colors
        ImGui::PushStyleColor(ImGuiCol_Button, color);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(color.x + BUTTON_HOVERED_LIGHTEN_BOOST, color.y + BUTTON_HOVERED_LIGHTEN_BOOST, color.z + BUTTON_HOVERED_LIGHTEN_BOOST, color.w));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(color.x + BUTTON_ACTIVE_LIGHTEN_BOOST, color.y + BUTTON_ACTIVE_LIGHTEN_BOOST, color.z + BUTTON_ACTIVE_LIGHTEN_BOOST, color.w));

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

        if (ImGui::Button(label, size))
        {
            log_filter_mask ^= static_cast<int>(log_level);
        }

        ImGui::PopStyleColor(4);
    }
}

Console::Console() : EditorPanel("Logger")
{
    m_flags |= ImGuiWindowFlags_HorizontalScrollbar;
}

void Console::begin_frame()
{
    EditorPanel::begin_frame();
}

void Console::update()
{
    EditorPanel::update();

    const string count_text = to_string(Logger::get_log_entries().size()) + "/" + to_string(MAX_LOG_ENTRIES);

    ImGui::SameLine();

    draw_log_filter_button("Info", LogLevel::Log_Info, BUTTON_SIZE, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));

    ImGui::SameLine();

    draw_log_filter_button("Warning", LogLevel::Log_Warning, BUTTON_SIZE, ImVec4(0.5f, 0.5f, 0.0f, 1.0f));

    ImGui::SameLine();

    draw_log_filter_button("Error", LogLevel::Log_Error, BUTTON_SIZE, ImVec4(0.7f, 0.0f, 0.0f, 1.0f));

    ImGui::SameLine();

    // Log count tracker
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
    const ImVec4 color = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
    ImGui::PushStyleColor(ImGuiCol_Button, color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);

    ImGui::Button(count_text.c_str(), BUTTON_SIZE);

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();

    ImGui::SameLine();

    // Clear button
    if (ImGui::Button("Clear", BUTTON_SIZE))
    {
        Logger::clear_log_entries();
    }

    ImGui::SameLine();

    // Search bar
    static char search_buffer[64] = "";
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputTextWithHint("##log_filter", "Filter logs by search...", search_buffer, IM_ARRAYSIZE(search_buffer));

    ImGuiB3D::SeparatorWithSpacing(1);

    // Log table entries
    ImGui::BeginTable("##console_text", 1, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_ScrollY);
    {
        int current_log_index = 0;
        for (const auto& log_entry : Logger::get_log_entries())
        {
            string full_log = log_entry.header_buffer + log_entry.log_buffer;
            if (search_buffer[0] != '\0' && !ImGuiB3D::StringContainsIgnoreCase(full_log, string(search_buffer)))
            {
                continue;
            }

            if (const int level_bit = static_cast<int>(log_entry.log_level); (level_bit & log_filter_mask) == 0)
            {
                continue;
            }
            
            ImGui::TableNextRow(ImGuiTableRowFlags_None, 25.0f);
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();

            ImVec4 log_color;
            switch (log_entry.log_level)
            {
                case LogLevel::Log_Warning: 
                    log_color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); 
                    break;
                case LogLevel::Log_Error: 
                    log_color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); 
                    break;
                case LogLevel::Log_Info:
                case LogLevel::Log_Max:
                default: 
                    log_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); 
                    break;
            }
            const string label = to_string(current_log_index++) + log_entry.header_buffer;
            ImGuiB3D::InteractableMultilineText(label, full_log, log_color);
        }

        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        {
            ImGui::SetScrollHereY(1.0f);
        }
    }
    ImGui::EndTable();
}

void Console::end_frame()
{
    EditorPanel::end_frame();
}
