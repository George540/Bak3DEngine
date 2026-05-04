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

#include "metrics.h"

#include <algorithm>
#include <imgui_impl_opengl3_loader.h>
#include <implot.h>
#include <vector>

#include "editor.h"
#include "GLFW/glfw3.h"
#include "Input/event_manager.h"

using namespace std;

struct FrameSample
{
    float time;  // editor lifetime timestamp in seconds
    float value; // frame time in ms
};

namespace
{
    ImPlotSpec plot_specs_ms;
    vector<FrameSample> frame_time_stamps_plotted;

    constexpr double SOFT_MS_THRESHOLD = 16.66;
    constexpr int MAX_PLOTS_COUNT = 100;
    constexpr float ZOOM_FACTOR = 0.1f; // percentage per scroll tick

    float history = 10.0f;
    float y_max   = 33.33f; // start at your current 30 FPS limit
}

Metrics::Metrics() : EditorPanel("Metrics")
{
    plot_specs_ms.Offset = 0;
}

void Metrics::begin_frame()
{
    EditorPanel::begin_frame();
}

void Metrics::update()
{
    EditorPanel::update();

    if (ImGui::BeginTable("##metrics_table", 2, ImGuiTableFlags_Resizable))
    {
        ImGui::TableSetupColumn("Metrics", ImGuiTableColumnFlags_WidthStretch, 0.65f);
        ImGui::TableSetupColumn("Hardware", ImGuiTableColumnFlags_WidthStretch, 0.35f);

        ImGui::TableNextColumn();
        update_metrics();

        ImGui::TableNextColumn();
        update_hardware();

        ImGui::EndTable();
    }
}

void Metrics::end_frame()
{
    EditorPanel::end_frame();
}

void Metrics::update_metrics()
{
    float mouse_wheel = ImGui::GetIO().MouseWheel;

    int fps_count = EventManager::get_frames_per_second();
    float frame_time_ms = 1000.0f * EventManager::get_frame_time();
    float imgui_frame_time_ms = ImGui::GetIO().Framerate;

    ImGui::Text("FPS: %d", fps_count);
    ImGui::Text("Frame Time: %.3f ms", frame_time_ms);
    ImGui::Text("ImGuiIO: %.0f FPS", imgui_frame_time_ms);

    // Stats Plot
    {
        ImGui::SliderFloat("History", &history, 10, 60, "%.1f s");

        float time_elapsed = Bak3DEditor::get_editor_lifetime();

        // Store timestamped samples
        frame_time_stamps_plotted.push_back({ time_elapsed, frame_time_ms });

        // Cull samples older than the history window
        while (!frame_time_stamps_plotted.empty() &&
               frame_time_stamps_plotted.front().time < time_elapsed - history)
        {
            frame_time_stamps_plotted.erase(frame_time_stamps_plotted.begin());
        }

        // Split into parallel arrays for ImPlot
        static std::vector<float> xs, ys;
        xs.clear();
        ys.clear();
        xs.reserve(frame_time_stamps_plotted.size());
        ys.reserve(frame_time_stamps_plotted.size());
        for (const auto& s : frame_time_stamps_plotted)
        {
            xs.push_back(s.time);
            ys.push_back(s.value);
        }

        if (ImPlot::BeginPlot("##UnitGraph", ImVec2(-1, ImGui::GetTextLineHeight() * 11)))
        {
            ImPlot::SetupAxes("Time (s)", "Stat Time (ms)", ImPlotAxisFlags_NoTickLabels, 0);
            ImPlot::SetupAxisLimits(ImAxis_X1, time_elapsed - history, time_elapsed, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1, 0, y_max, ImGuiCond_Always);

            ImPlot::PlotLine("Frame Time",
                xs.data(), ys.data(),
                static_cast<int>(xs.size()),
                plot_specs_ms);

            ImPlot::PlotInfLines("##60 FPS", &SOFT_MS_THRESHOLD, 1,
            {
                ImPlotProp_LineColor, ImVec4(1.0f, 0.4f, 0.1f, 1.0f),
                ImPlotProp_Flags, ImPlotInfLinesFlags_Horizontal
            });

            ImPlot::EndPlot();
        }

        // Zoom Y axis with mouse wheel when hovering the plot
        if (ImGui::IsItemHovered())
        {
            if (mouse_wheel != 0.0f)
            {
                y_max *= (1.0f - mouse_wheel * ZOOM_FACTOR);
                y_max = clamp(y_max, 1.0f, 500.0f); // sane limits: 1ms to 500ms
            }
        }
    }
}

void Metrics::update_hardware()
{
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Hardware"))
    {
        const GLubyte* vendor = glGetString(GL_VENDOR);
        const GLubyte* renderer = glGetString(GL_RENDERER);
        const GLFWvidmode* mode = EventManager::get_vid_mode();

        ImGui::Text("Vendor: %s", vendor);
        ImGui::Text("Renderer: %s", renderer);
        ImGui::Text("Display Resolution: %d x %d", mode->width, mode->height);
        ImGui::Text("Refresh Rate: %d Hz", mode->refreshRate);
        ImGui::Text("Color Depth: R%d G%d B%d", mode->redBits, mode->greenBits, mode->blueBits);

        ImGui::TreePop();
    }
}
