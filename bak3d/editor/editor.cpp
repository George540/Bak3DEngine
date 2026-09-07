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

#include "editor.h"

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>
#include <implot.h>
#include <iostream>
#include <ranges>

#include "toolbar.h"
#include "Core/logger.h"
#include "Panel/asset_viewer.h"
#include "Panel/console.h"
#include "Panel/details.h"
#include "Panel/editor_panel.h"
#include "Panel/environment.h"
#include "Panel/metrics.h"
#include "Panel/scene_graph.h"
#include "Panel/splash_screen.h"
#include "Panel/viewport.h"
#include "Renderer/renderer.h"

using namespace std;

namespace
{
    map<string, shared_ptr<EditorPanel>> m_panels;
    const char* editor_space_name = "##editor_window";
    float editor_time_elapsed = 0;
}

constexpr auto window_flags =
        ImGuiWindowFlags_NoDocking             |
        ImGuiWindowFlags_NoTitleBar            |
        ImGuiWindowFlags_NoCollapse            |
        ImGuiWindowFlags_NoResize              |
        ImGuiWindowFlags_NoMove                |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus            |
        ImGuiWindowFlags_NoBackground          |
        ImGuiWindowFlags_NoScrollbar           |
        ImGuiWindowFlags_NoScrollWithMouse;

void Bak3DEditor::initialize()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    // Configure ImGui
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = "editor_config.ini";
    io.ConfigWindowsResizeFromEdges = true;
    io.ConfigDragClickToInputText = true;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard
                   | ImGuiConfigFlags_DockingEnable
                   | ImGuiConfigFlags_ViewportsEnable
                   | ImGuiConfigFlags_NoMouseCursorChange;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(Renderer::get_window(), true);
    const auto glsl_version = "#version 460";
    ImGui_ImplOpenGL3_Init(glsl_version);

    m_panels["Viewport"] = make_shared<Viewport>();
    m_panels["Environment"] = make_shared<Environment>();
    m_panels["Details"] = make_shared<Details>();
    m_panels["Scene"] = make_shared<SceneGraph>();
    m_panels["Assets"] = make_shared<AssetPanel>();
    m_panels["Metrics"] = make_shared<Metrics>();
    m_panels["Console"] = make_shared<Console>();
    m_panels["Splash Screen"] = make_shared<SplashScreen>();

    set_editor_global_style();

    B3D_LOG_INFO("ImGui and editor initialized.");
}

void Bak3DEditor::update()
{
    begin_frame();

    update_window();

    end_frame();
}

void Bak3DEditor::shutdown()
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
}

float Bak3DEditor::get_editor_lifetime()
{
    return editor_time_elapsed;
}

map<string, shared_ptr<EditorPanel>> Bak3DEditor::get_panels()
{
    return m_panels;
}

shared_ptr<EditorPanel> Bak3DEditor::get_panel(const string& name)
{
    if (const auto it = m_panels.find(string(name)); it != m_panels.end())
    {
        return it->second;
    }
    return nullptr;
}

void Bak3DEditor::begin_frame()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Bak3DEditor::end_frame()
{
    // Render ImGui window result
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    // Update child windows
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void Bak3DEditor::set_editor_global_style()
{
    set_dark_pastel_style();
}

void Bak3DEditor::set_dark_pastel_style()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Backgrounds 
    colors[ImGuiCol_WindowBg]           = ImVec4(0.12f, 0.13f, 0.15f, 1.00f); // Dark grey base
    colors[ImGuiCol_ChildBg]            = ImVec4(0.14f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_PopupBg]            = ImVec4(0.10f, 0.10f, 0.12f, 0.95f);
    colors[ImGuiCol_Border]             = ImVec4(0.30f, 0.33f, 0.42f, 0.40f);

    //Text 
    colors[ImGuiCol_Text]               = ImVec4(0.90f, 0.93f, 0.95f, 1.00f);
    colors[ImGuiCol_TextDisabled]       = ImVec4(0.60f, 0.65f, 0.70f, 1.00f);

    // Headers 
    colors[ImGuiCol_Header]             = ImVec4(0.36f, 0.42f, 0.55f, 0.60f);
    colors[ImGuiCol_HeaderHovered]      = ImVec4(0.44f, 0.50f, 0.68f, 0.80f);
    colors[ImGuiCol_HeaderActive]       = ImVec4(0.46f, 0.55f, 0.75f, 1.00f);

    // Buttons 
    colors[ImGuiCol_Button]             = ImVec4(0.28f, 0.34f, 0.48f, 0.70f);
    colors[ImGuiCol_ButtonHovered]      = ImVec4(0.36f, 0.45f, 0.65f, 0.85f);
    colors[ImGuiCol_ButtonActive]       = ImVec4(0.40f, 0.50f, 0.70f, 1.00f);

    // Frames 
    colors[ImGuiCol_FrameBg]            = ImVec4(0.20f, 0.22f, 0.28f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]     = ImVec4(0.28f, 0.32f, 0.42f, 1.00f);
    colors[ImGuiCol_FrameBgActive]      = ImVec4(0.32f, 0.38f, 0.50f, 1.00f);

    // Tabs 
    colors[ImGuiCol_Tab]                = ImVec4(0.26f, 0.30f, 0.42f, 0.80f);
    colors[ImGuiCol_TabHovered]         = ImVec4(0.36f, 0.42f, 0.58f, 1.00f);
    colors[ImGuiCol_TabActive]          = ImVec4(0.42f, 0.50f, 0.68f, 1.00f);
    colors[ImGuiCol_TabUnfocused]       = ImVec4(0.20f, 0.24f, 0.32f, 0.80f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.30f, 0.36f, 0.50f, 1.00f);

    // Titles 
    colors[ImGuiCol_TitleBg]            = ImVec4(0.20f, 0.25f, 0.30f, 1.00f);
    colors[ImGuiCol_TitleBgActive]      = ImVec4(0.25f, 0.30f, 0.40f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]   = ImVec4(0.10f, 0.12f, 0.15f, 0.75f);

    // Scrollbars 
    colors[ImGuiCol_ScrollbarBg]        = ImVec4(0.13f, 0.14f, 0.18f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]      = ImVec4(0.25f, 0.30f, 0.38f, 0.60f);
    colors[ImGuiCol_ScrollbarGrabHovered]=ImVec4(0.35f, 0.40f, 0.50f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabActive]= ImVec4(0.45f, 0.50f, 0.65f, 1.00f);

    // Checkboxes / Radios 
    colors[ImGuiCol_CheckMark]          = ImVec4(0.80f, 0.85f, 1.00f, 1.00f);

    // Sliders 
    colors[ImGuiCol_SliderGrab]         = ImVec4(0.50f, 0.65f, 0.90f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]   = ImVec4(0.60f, 0.75f, 1.00f, 1.00f);

    // Resize Grip 
    colors[ImGuiCol_ResizeGrip]         = ImVec4(0.30f, 0.40f, 0.50f, 0.60f);
    colors[ImGuiCol_ResizeGripHovered]  = ImVec4(0.40f, 0.50f, 0.60f, 0.80f);
    colors[ImGuiCol_ResizeGripActive]   = ImVec4(0.50f, 0.60f, 0.80f, 1.00f);

    // Separator 
    colors[ImGuiCol_Separator]          = ImVec4(0.35f, 0.40f, 0.48f, 0.7f);
    colors[ImGuiCol_SeparatorHovered]   = ImVec4(0.50f, 0.60f, 0.72f, 0.9f);
    colors[ImGuiCol_SeparatorActive]    = ImVec4(0.65f, 0.70f, 0.85f, 1.0f);

    // Menus and Tooltips 
    colors[ImGuiCol_MenuBarBg]          = ImVec4(0.14f, 0.15f, 0.17f, 1.00f);
    // colors[ImGuiCol_TooltipBg]          = ImVec4(0.18f, 0.20f, 0.25f, 0.95f);

    // Drag & Drop 
    colors[ImGuiCol_DragDropTarget]     = ImVec4(0.50f, 0.85f, 1.00f, 0.90f);

    // Style Metrics 
    style.WindowRounding    = 8.0f;
    style.ChildRounding     = 6.0f;
    style.FrameRounding     = 5.0f;
    style.PopupRounding     = 6.0f;
    style.ScrollbarRounding = 5.0f;
    style.GrabRounding      = 4.0f;
    style.TabRounding       = 5.0f;

    style.WindowBorderSize  = 0.0f;
    style.FrameBorderSize   = 0.0f;
    style.PopupBorderSize   = 1.0f;

    //style.WindowPadding     = ImVec2(16, 16);
    //style.FramePadding      = ImVec2(10, 6);
    //style.ItemSpacing       = ImVec2(10, 10);
    //style.ItemInnerSpacing  = ImVec2(6, 4);
    style.IndentSpacing     = 20.0f;
}

void Bak3DEditor::update_window()
{
    editor_time_elapsed += ImGui::GetIO().DeltaTime;
    // 1. Set Main Viewport properties and style
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    Toolbar::draw_toolbar();

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    // 2. Begin Window
    ImGui::Begin(editor_space_name, nullptr, window_flags);
    ImGui::PopStyleVar(3);

    // 3. Update dock space panels
    update_panels(viewport);

    // Show demo window when required
    //ImGui::ShowDemoWindow();

    // 5. End main window and frame
    ImGui::End();
}

void Bak3DEditor::update_panels(const ImGuiViewport* viewport)
{
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        const auto window_id = ImGui::GetID(editor_space_name);
        if (!ImGui::DockBuilderGetNode(window_id))
        {
            ImGui::DockBuilderRemoveNode(window_id);
            ImGui::DockBuilderAddNode(window_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(window_id, viewport->WorkSize);

            ImGuiID dock_main_id = window_id;

            // 1. Isolate Details on the far right for Details (Full height)
            ImGuiID dock_id_left_container;
            const ImGuiID dock_id_environment = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.23f, nullptr, &dock_id_left_container);

            // 2. Split the left container horizontally for Viewport (Right or Middle) and Details (Right). This creates a bottom row that spans under everything except Details
            ImGuiID dock_id_bottom_row;
            const ImGuiID dock_id_top_row = ImGui::DockBuilderSplitNode(dock_id_left_container, ImGuiDir_Up, 0.72f, nullptr, &dock_id_bottom_row);

            // 3. Split the top row vertically for Scene (Left) and Viewport (Right)
            ImGuiID dock_id_viewport;
            const ImGuiID dock_id_scene = ImGui::DockBuilderSplitNode(dock_id_top_row, ImGuiDir_Left, 0.15f, nullptr, &dock_id_viewport);

            // 4. Split the bottom row vertically for Logger (Left) and Assets (Right)
            ImGuiID dock_id_assets;
            const ImGuiID dock_id_logger = ImGui::DockBuilderSplitNode(dock_id_bottom_row, ImGuiDir_Left, 0.4f, nullptr, &dock_id_assets);

            // 5. Assign Windows and finish
            ImGui::DockBuilderDockWindow("Viewport", dock_id_viewport);
            ImGui::DockBuilderDockWindow("Environment", dock_id_environment);
            ImGui::DockBuilderDockWindow("Details", dock_id_environment);
            ImGui::DockBuilderDockWindow("Scene", dock_id_scene);
            ImGui::DockBuilderDockWindow("Logger", dock_id_logger);
            ImGui::DockBuilderDockWindow("Assets", dock_id_assets);
            ImGui::DockBuilderDockWindow("Metrics", dock_id_assets);

            // Note: to reset the panels before runtime, simply delete the editor_config.ini file found inside the build directory.
            ImGui::DockBuilderFinish(window_id);
        }

        // 6. Finish styling
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f); // Thin border for the splitter look
        ImGui::DockSpace(window_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::PopStyleVar(2);
    }

    // 7. Update all panels if visible
    for (const auto& panel : m_panels | views::values)
    {
        panel->begin_frame();
        panel->update();
        panel->end_frame();
    }
}
