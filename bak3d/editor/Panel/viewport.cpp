#include "viewport.h"

#include <imgui_internal.h>
#include <glm/common.hpp>

#include "Core/global_settings.h"
#include "Input/event_manager.h"
#include "Renderer/post_processor.h"
#include "Renderer/renderer.h"

namespace
{
    ImVec2 previous_viewport_size = ImVec2(0, 0);
    ImVec2 viewport_panel_size = ImVec2(0, 0);
}

Viewport::Viewport() : EditorPanel("Viewport")
{
    m_flags |= ImGuiWindowFlags_NoScrollbar;
}

void Viewport::begin_frame()
{
    EditorPanel::begin_frame();
}

void Viewport::update()
{
    EditorPanel::update();

    viewport_panel_size = ImGui::GetContentRegionAvail();

    // Guard against zero or near-zero dimensions during resize
    if (viewport_panel_size.x < 1.0f || viewport_panel_size.y < 1.0f)
    {
        return;
    }

    const bool post_processing_enabled = GlobalSettings::get_global_setting_value<bool>(GlobalSettingOption::PostProcessing_Enabled);
    const auto view_mode = static_cast<DebugViewMode>(GlobalSettings::get_global_setting_value<int>(GlobalSettingOption::ViewSelection));

    const FrameBuffer* frame_buffer_main =
        view_mode != DebugViewMode::Default
        ? Renderer::get_debug_view_buffer()
        : Renderer::get_main_frame_buffer();

    const float fb_aspect = frame_buffer_main->get_aspect_ratio();
    const float view_aspect = viewport_panel_size.x / viewport_panel_size.y;

    // Start with Y flipped: top is 1.0f, bottom is 0.0f
    ImVec2 uv0(0.0f, 1.0f);
    ImVec2 uv1(1.0f, 0.0f);

    if (view_aspect > fb_aspect)
    {
        // Viewport is wider than the image: Crop the top and bottom
        const float scale = fb_aspect / view_aspect;
        const float delta = (1.0f - scale) * 0.5f;

        // Offset from the flipped baseline
        uv0.y = 1.0f - delta; // Start slightly below the top (1.0)
        uv1.y = delta;        // End slightly above the bottom (0.0)
    }
    else
    {
        // Viewport is taller than the image: Crop the sides instead
        const float scale = view_aspect / fb_aspect;
        const float delta = (1.0f - scale) * 0.5f;

        uv0.x = delta;
        uv1.x = 1.0f - delta;
    }

    // Track whether the drag originated from the viewport window
    static bool drag_started_in_viewport = false;

    const bool is_window_resizing = ImGui::IsMouseDragging(ImGuiMouseButton_Left)
                                  && (glm::abs(previous_viewport_size.x - viewport_panel_size.x) > 0.001f 
                                  || glm::abs(previous_viewport_size.y - viewport_panel_size.y) > 0.001f);

    // Latch: only set true when the click begins in the viewport
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
    {
        drag_started_in_viewport = true;
    }

    // Release the latch when mouse is released
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    {
        drag_started_in_viewport = false;
    }

    EventManager::is_dragging_enabled = drag_started_in_viewport && !is_window_resizing;
    EventManager::is_scrolling_enabled = ImGui::IsWindowHovered();

    void* viewport_texture = reinterpret_cast<void*>(static_cast<intptr_t>(frame_buffer_main->get_color_texture()));
    ImGui::Image(viewport_texture, viewport_panel_size, uv0, uv1);

    previous_viewport_size = viewport_panel_size;
    EventManager::set_viewport_width(viewport_panel_size.x);
    EventManager::set_viewport_height(viewport_panel_size.y);

    bool isHovered = ImGui::IsItemHovered();
}

void Viewport::end_frame()
{
    EditorPanel::end_frame();
}

float Viewport::get_viewport_width() const
{
    return viewport_panel_size.x;
}

float Viewport::get_viewport_height() const
{
    return viewport_panel_size.y;
}

float Viewport::get_viewport_aspect_ratio() const
{
    return viewport_panel_size.x;
}
