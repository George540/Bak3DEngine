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

#include "toolbar.h"

#include <imgui.h>

#include "editor.h"
#include "Asset/resource_manager.h"

void Toolbar::draw_toolbar()
{
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::GetStyle().Colors[ImGuiCol_MenuBarBg]);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    if (ImGui::BeginMainMenuBar())
    {
        draw_engine_icon();

        ImGui::Separator();

        draw_file_menu();
        draw_view_menu();
        draw_help_menu();

        ImGui::Separator();

        draw_simulation_buttons();

        ImGui::EndMainMenuBar();
    }

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}

void Toolbar::draw_engine_icon()
{
    const ImTextureID engine_icon_id = ResourceManager::get_texture("bak3d_icon.png")->get_texture_id();
    ImGui::Image(engine_icon_id, ImVec2(20.0f, 20.0f), ImVec2(0, 1), ImVec2(1, 0));

    ImGui::TextUnformatted("Bak3D Engine v1.3");
}

void Toolbar::draw_file_menu()
{
    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("New Scene"))
        {
            
        }
        if (ImGui::MenuItem("Open Scene"))
        {
            
        }
        if (ImGui::MenuItem("Save Scene"))
        {
            
        }
        if (ImGui::MenuItem("Save Project"))
        {
            
        }
        ImGui::EndMenu();
    }
}

void Toolbar::draw_view_menu()
{
    if (ImGui::BeginMenu("Window"))
    {
        for (const auto& [name, panel] : Bak3DEditor::get_panels())
        {
            if (name == "Splash Screen")
            {
                continue;
            }

            if (ImGui::MenuItem(panel->get_editor_panel_title()))
            {
            
            }
        }

        ImGui::EndMenu();
    }
}

void Toolbar::draw_help_menu()
{
    if (ImGui::BeginMenu("Help"))
    {
        if (ImGui::MenuItem("Repository"))
        {
            
        }
        if (ImGui::MenuItem("Support"))
        {
            
        }
        if (ImGui::MenuItem("Splash Screen"))
        {
            const auto panel = Bak3DEditor::get_panel("Splash Screen");
            panel->set_visible(true);
        }

        ImGui::EndMenu();
    }
}

void Toolbar::draw_simulation_buttons()
{
    const ImGuiStyle& style = ImGui::GetStyle();

    const float play_width = ImGui::CalcTextSize("Play").x + style.FramePadding.x * 2.0f;
    const float pause_width = ImGui::CalcTextSize("Pause").x + style.FramePadding.x * 2.0f;
    const float end_width = ImGui::CalcTextSize("End").x + style.FramePadding.x * 2.0f;
    
    // Total width is the sum of all buttons plus the item spacing between them (2 gaps)
    const float total_buttons_width = play_width + pause_width + end_width + (style.ItemSpacing.x * 2.0f);
    const float toolbar_width = ImGui::GetWindowWidth();
    const float start_cursor_x = (toolbar_width - total_buttons_width) * 0.5f;

    // Force the cursor to start printing at our calculated center-aligned position
    if (start_cursor_x > ImGui::GetCursorPosX())
    {
        ImGui::SetCursorPosX(start_cursor_x);
    }

    if (ImGui::Button("Play"))
    {
        /* Start simulation */
    }

    ImGui::SameLine();

    if (ImGui::Button("Pause"))
    {
        /* Pause simulation */
    }

    ImGui::SameLine();

    if (ImGui::Button("End"))
    {
        /* End simulation */
    }
}
