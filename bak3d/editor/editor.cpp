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

    constexpr auto rgb = [](const float r, const float g, const float b, const float a = 1.0f) { return ImVec4(r, g, b, a); };

    // Elevation
    const ImVec4 inset       = rgb(0.075f, 0.083f, 0.110f);        // recessed: inputs, scrollbar track
    const ImVec4 base        = rgb(0.098f, 0.108f, 0.140f);        // windows
    const ImVec4 surface     = rgb(0.125f, 0.137f, 0.172f);        // child regions, menu bar
    const ImVec4 raised      = rgb(0.190f, 0.212f, 0.280f);        // buttons, headers
    const ImVec4 raised_hov  = rgb(0.240f, 0.270f, 0.355f);
    const ImVec4 raised_act  = rgb(0.290f, 0.330f, 0.435f);
    // Accent
    const ImVec4 accent      = rgb(0.540f, 0.680f, 0.950f);
    const ImVec4 accent_hov  = rgb(0.640f, 0.760f, 1.000f);
    const ImVec4 accent_dim  = rgb(0.540f, 0.680f, 0.950f, 0.35f);
    // Edges
    const ImVec4 border      = rgb(0.330f, 0.380f, 0.500f, 0.45f); // bevel highlight
    const ImVec4 border_soft = rgb(0.330f, 0.380f, 0.500f, 0.25f);

    // Text
    colors[ImGuiCol_Text]                  = rgb(0.900f, 0.925f, 0.960f);
    colors[ImGuiCol_TextDisabled]          = rgb(0.520f, 0.570f, 0.650f);
    colors[ImGuiCol_TextSelectedBg]        = accent_dim;
    colors[ImGuiCol_TextLink]              = accent;

    // Surfaces
    colors[ImGuiCol_WindowBg]              = base;
    colors[ImGuiCol_ChildBg]               = surface;
    colors[ImGuiCol_PopupBg]               = rgb(0.085f, 0.094f, 0.125f, 0.97f);
    colors[ImGuiCol_MenuBarBg]             = surface;
    colors[ImGuiCol_ModalWindowDimBg]      = rgb(0.03f, 0.04f, 0.06f, 0.60f);

    // Borders
    colors[ImGuiCol_Border]                = border;
    colors[ImGuiCol_BorderShadow]          = rgb(0.0f, 0.0f, 0.0f, 0.0f);

    // Inputs (inset)
    colors[ImGuiCol_FrameBg]               = inset;
    colors[ImGuiCol_FrameBgHovered]        = rgb(0.110f, 0.122f, 0.165f);
    colors[ImGuiCol_FrameBgActive]         = rgb(0.135f, 0.152f, 0.210f);

    // Buttons (raised)
    colors[ImGuiCol_Button]                = raised;
    colors[ImGuiCol_ButtonHovered]         = raised_hov;
    colors[ImGuiCol_ButtonActive]          = raised_act;

    // Headers / selectables / tree nodes
    colors[ImGuiCol_Header]                = rgb(0.190f, 0.212f, 0.280f, 0.65f);
    colors[ImGuiCol_HeaderHovered]         = rgb(0.260f, 0.300f, 0.400f, 0.80f);
    colors[ImGuiCol_HeaderActive]          = rgb(0.300f, 0.350f, 0.470f, 1.00f);

    // Tabs (selected tab gets an accent overline for clear focus)
    colors[ImGuiCol_Tab]                   = rgb(0.120f, 0.132f, 0.170f);
    colors[ImGuiCol_TabHovered]            = raised_hov;
    colors[ImGuiCol_TabSelected]           = rgb(0.185f, 0.205f, 0.270f);
    colors[ImGuiCol_TabSelectedOverline]   = accent;
    colors[ImGuiCol_TabDimmed]             = rgb(0.100f, 0.110f, 0.145f);
    colors[ImGuiCol_TabDimmedSelected]     = rgb(0.150f, 0.166f, 0.220f);
    colors[ImGuiCol_TabDimmedSelectedOverline] = accent_dim;

    // Title bars (docked windows use the tab bar, floating ones use these)
    colors[ImGuiCol_TitleBg]               = rgb(0.080f, 0.088f, 0.118f);
    colors[ImGuiCol_TitleBgActive]         = rgb(0.130f, 0.145f, 0.195f);
    colors[ImGuiCol_TitleBgCollapsed]      = rgb(0.080f, 0.088f, 0.118f, 0.75f);

    // Scrollbars
    colors[ImGuiCol_ScrollbarBg]           = rgb(0.075f, 0.083f, 0.110f, 0.60f);
    colors[ImGuiCol_ScrollbarGrab]         = rgb(0.250f, 0.290f, 0.380f);
    colors[ImGuiCol_ScrollbarGrabHovered]  = rgb(0.330f, 0.380f, 0.490f);
    colors[ImGuiCol_ScrollbarGrabActive]   = accent;

    // Accent controls
    colors[ImGuiCol_CheckMark]             = accent_hov;
    colors[ImGuiCol_SliderGrab]            = accent;
    colors[ImGuiCol_SliderGrabActive]      = accent_hov;

    // Resize grips & separators
    colors[ImGuiCol_ResizeGrip]            = rgb(0.330f, 0.380f, 0.500f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered]     = accent_dim;
    colors[ImGuiCol_ResizeGripActive]      = accent;
    colors[ImGuiCol_Separator]             = border_soft;
    colors[ImGuiCol_SeparatorHovered]      = accent_dim;
    colors[ImGuiCol_SeparatorActive]       = accent;

    // Tables
    colors[ImGuiCol_TableHeaderBg]         = rgb(0.140f, 0.155f, 0.205f);
    colors[ImGuiCol_TableBorderStrong]     = border;
    colors[ImGuiCol_TableBorderLight]      = border_soft;
    colors[ImGuiCol_TableRowBg]            = rgb(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_TableRowBgAlt]         = rgb(0.5f, 0.6f, 0.9f, 0.035f); // faint zebra striping

    // Docking & drag/drop & nav
    colors[ImGuiCol_DockingPreview]        = accent_dim;
    colors[ImGuiCol_DockingEmptyBg]        = rgb(0.060f, 0.066f, 0.090f);
    colors[ImGuiCol_DragDropTarget]        = accent_hov;
    colors[ImGuiCol_NavCursor]             = accent;
    colors[ImGuiCol_NavWindowingHighlight] = rgb(1.0f, 1.0f, 1.0f, 0.60f);
    colors[ImGuiCol_NavWindowingDimBg]     = rgb(0.03f, 0.04f, 0.06f, 0.50f);

    // Plots (ImPlot picks these up for frame time graph)
    colors[ImGuiCol_PlotLines]             = accent;
    colors[ImGuiCol_PlotLinesHovered]      = rgb(1.0f, 0.55f, 0.30f);
    colors[ImGuiCol_PlotHistogram]         = accent;
    colors[ImGuiCol_PlotHistogramHovered]  = accent_hov;

    // Rounding
    style.WindowRounding    = 6.0f;
    style.ChildRounding     = 5.0f;
    style.FrameRounding     = 4.0f;
    style.PopupRounding     = 6.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding      = 3.0f;
    style.TabRounding       = 5.0f;

    // Borders
    style.WindowBorderSize  = 1.0f;
    style.ChildBorderSize   = 1.0f;
    style.PopupBorderSize   = 1.0f;
    style.FrameBorderSize   = 1.0f;
    style.TabBorderSize     = 0.0f;
    style.TabBarBorderSize  = 1.0f;
    style.TabBarOverlineSize = 2.0f;
    style.DockingSeparatorSize = 2.0f;
    style.SeparatorTextBorderSize = 1.0f;
    style.SeparatorTextAlign   = ImVec2(0.0f, 0.5f);
    style.SeparatorTextPadding = ImVec2(20.0f, 4.0f);

    // Spacing
    style.WindowPadding     = ImVec2(10.0f, 10.0f);
    style.FramePadding      = ImVec2(8.0f, 4.0f);
    style.ItemSpacing       = ImVec2(8.0f, 6.0f);
    style.ItemInnerSpacing  = ImVec2(6.0f, 4.0f);
    style.CellPadding       = ImVec2(6.0f, 4.0f);
    style.IndentSpacing     = 20.0f;
    style.ScrollbarSize     = 12.0f;
    style.GrabMinSize       = 10.0f;
    style.WindowMenuButtonPosition = ImGuiDir_None; // hides the collapse arrow (panels are NoCollapse)
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

            const ImGuiID dock_main_id = window_id;

            // 1. Isolate the right column (full height) for Scene + Environment/Details
            ImGuiID dock_id_left_container;
            const ImGuiID dock_id_right_container = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.3f, nullptr, &dock_id_left_container);

            // 1b. Split the right column vertically: Scene on top, Environment+Details (tabbed) below
            ImGuiID dock_id_environment;
            const ImGuiID dock_id_scene = ImGui::DockBuilderSplitNode(dock_id_right_container, ImGuiDir_Up, 0.33f, nullptr, &dock_id_environment);

            // 2. Split the left container horizontally for Viewport (top row, full width now) and bottom row (Logger/Assets)
            ImGuiID dock_id_bottom_row;
            const ImGuiID dock_id_viewport = ImGui::DockBuilderSplitNode(dock_id_left_container, ImGuiDir_Up, 0.72f, nullptr, &dock_id_bottom_row);

            // 3. Split the bottom row vertically for Logger (Left) and Assets (Right)
            ImGuiID dock_id_assets;
            const ImGuiID dock_id_logger = ImGui::DockBuilderSplitNode(dock_id_bottom_row, ImGuiDir_Left, 0.4f, nullptr, &dock_id_assets);

            // 4. Assign Windows and finish
            ImGui::DockBuilderDockWindow("Viewport", dock_id_viewport);
            ImGui::DockBuilderDockWindow("Scene", dock_id_scene);
            ImGui::DockBuilderDockWindow("Environment", dock_id_environment);
            ImGui::DockBuilderDockWindow("Details", dock_id_environment);
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
