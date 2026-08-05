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

#include "Core/logger.h"
#include "Panel/asset_viewer.h"
#include "Panel/console.h"
#include "Panel/details.h"
#include "Panel/editor_panel.h"
#include "Panel/environment.h"
#include "Panel/metrics.h"
#include "Panel/scene_graph.h"
#include "Panel/viewport.h"
#include "Renderer/renderer.h"

using namespace std;

namespace
{
    std::vector<std::shared_ptr<EditorPanel>> m_panels;
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
        ImGuiWindowFlags_NoBackground;

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

    m_panels.emplace_back(make_shared<Viewport>());
    m_panels.emplace_back(make_shared<Environment>());
    m_panels.emplace_back(make_shared<Details>());
    m_panels.emplace_back(make_shared<SceneGraph>());
    m_panels.emplace_back(make_shared<AssetPanel>());
    m_panels.emplace_back(make_shared<Metrics>());
    m_panels.emplace_back(make_shared<Console>());

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

void Bak3DEditor::update_window()
{
    editor_time_elapsed += ImGui::GetIO().DeltaTime;
    // 1. Set Main Viewport properties and style
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
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
    for (const shared_ptr<EditorPanel>& panel : m_panels)
    {
        panel->begin_frame();
        panel->update();
        panel->end_frame();
    }
}
