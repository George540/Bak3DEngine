#include "viewport.h"

#include <imgui_internal.h>
#include <glm/common.hpp>

#include "imgui_b3d_extensions.h"
#include "Core/global_settings.h"
#include "Input/event_manager.h"
#include "Renderer/post_processor.h"
#include "Renderer/renderer.h"

using namespace std;

namespace
{
    ImVec2 previous_viewport_size = ImVec2(0, 0);
    ImVec2 viewport_panel_size = ImVec2(0, 0);
    bool m_camera_look_started_in_viewport = false;

    OverlaysFlags current_overlay_flags = OverlaysFlags::All;

    vector<OverlayFlagsDefinition> overlay_flags_list;
}

Viewport::Viewport() : EditorPanel("Viewport")
{
    m_flags |= ImGuiWindowFlags_NoScrollbar;

    overlay_flags_list =
    {
        {.flag = OverlaysFlags::WorldGrid, .label = "Grid" },
        {.flag = OverlaysFlags::LightIcons, .label = "Light Icons" }
    };
}

void Viewport::begin_frame()
{
    EditorPanel::begin_frame();
}

void Viewport::update()
{
    EditorPanel::update();

    ImGuiB3D::SeparatorWithSpacing();

    draw_viewport();
    sync_camera_controls();
}

void Viewport::end_frame()
{
    EditorPanel::end_frame();
}

void Viewport::draw_viewport()
{
    viewport_panel_size = ImGui::GetContentRegionAvail();

    // Guard against zero or near-zero dimensions during resize.
    if (viewport_panel_size.x < 1.0f || viewport_panel_size.y < 1.0f)
    {
        EventManager::set_camera_looking(false);
        return;
    }

    // Determine framebuffer to display
    const auto view_mode = static_cast<DebugViewMode>(GlobalSettings::get_global_setting_value<int>(GlobalSettingOption::VisualMode));
    const FrameBuffer* frame_buffer_main = view_mode != DebugViewMode::Lit
                                            ? Renderer::get_debug_view_buffer()
                                            : Renderer::get_main_frame_buffer();

    // Aspect ratio / UV cropping
    const float fb_aspect = frame_buffer_main->get_aspect_ratio();
    const float view_aspect = viewport_panel_size.x / viewport_panel_size.y;

    // Start with Y flipped: top = 1.0, bottom = 0.0
    ImVec2 uv0(0.0f, 1.0f);
    ImVec2 uv1(1.0f, 0.0f);

    if (view_aspect > fb_aspect)
    {
        // Viewport is wider than the image. Crop top and bottom.
        const float scale = fb_aspect / view_aspect;
        const float delta = (1.0f - scale) * 0.5f;
        uv0.y = 1.0f - delta;
        uv1.y = delta;
    }
    else
    {
        // Viewport is taller than the image. Crop left and right.
        const float scale = view_aspect / fb_aspect;
        const float delta = (1.0f - scale) * 0.5f;
        uv0.x = delta;
        uv1.x = 1.0f - delta;
    }

    // Display framebuffer
    void* viewport_texture = reinterpret_cast<void*>(static_cast<intptr_t>(frame_buffer_main->get_color_texture()));
    ImGui::Image(
        viewport_texture,
        viewport_panel_size,
        uv0,
        uv1
    );

    // Viewport dimensions syncing
    previous_viewport_size = viewport_panel_size;
    EventManager::set_viewport_width(viewport_panel_size.x);
    EventManager::set_viewport_height(viewport_panel_size.y);
}

void Viewport::sync_camera_controls()
{
    const bool viewport_hovered = ImGui::IsItemHovered();
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && viewport_hovered) // RMB was pressed while the viewport image was hovered
    {
        m_camera_look_started_in_viewport = true;
    }
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) // RMB was released anywhere
    {
        m_camera_look_started_in_viewport = false;
    }
    EventManager::set_scrolling_enabled(viewport_hovered);
    EventManager::set_camera_looking(m_camera_look_started_in_viewport);
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

void Viewport::draw_toolbar()
{
    draw_visual_modes_selection();

    ImGui::SameLine();

    draw_editor_overlays_selection();

    ImGui::SameLine();
}

void Viewport::draw_visual_modes_selection()
{
    if (ImGui::Button("Visual Modes"))
    {
        ImGui::OpenPopup("Visual Modes Popup");
    }

    ImGui::SetNextWindowSize(ImVec2(250.0f, 0.0f));
    if (ImGui::BeginPopup("Visual Modes Popup"))
    {
        ImGui::SeparatorText("Visual Modes");

        int view_selection = GlobalSettings::get_global_setting_value<int>(GlobalSettingOption::VisualMode);
        PagesData pages_data = Renderer::get_pages_data();

        // Initialize Table
        if (ImGui::BeginTable("VisualModesTable", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoSavedSettings))
        {
            // Setup columns: Left holds selection, Right holds settings
            ImGui::TableSetupColumn("Selection", ImGuiTableColumnFlags_WidthStretch, 0.4f);
            ImGui::TableSetupColumn("Settings", ImGuiTableColumnFlags_WidthStretch, 0.6f);

            for (int view_mode_num = 0; view_mode_num < static_cast<int>(DebugViewMode::Count); ++view_mode_num)
            {
                const auto view_mode = static_cast<DebugViewMode>(view_mode_num);
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::RadioButton(to_string(view_mode), &view_selection, view_mode_num);

                ImGui::TableSetColumnIndex(1);

                if (view_mode_num == static_cast<int>(DebugViewMode::Depth))
                {
                    const bool is_disabled = (view_selection != view_mode_num);
                    if (is_disabled) 
                    { 
                        ImGui::BeginDisabled(); 
                    }

                    ImGui::PushItemWidth(60.0f); // Slightly wider item width for the sliders

                    // Option 1: Near Distance
                    ImGui::Text("Near");
                    ImGui::SameLine();
                    ImGui::SliderFloat("##Near", &pages_data.depth_settings.r, 0.1f, pages_data.depth_settings.g - 0.01f, "%.1f");
                    if (ImGui::IsItemHovered())
                    {
                        ImGuiB3D::ToolTipExtendedText("Set the depth testing near distance with black color.\nThis determines how near the pixel is to the viewpoint.", TOOL_TIP_WIDTH);
                    }

                    // Option 2: Far Distance
                    ImGui::Spacing(); 
                    ImGui::Text("Far "); // Added space to match alignment of "Near"
                    ImGui::SameLine();
                    ImGui::SliderFloat("##Far", &pages_data.depth_settings.g, pages_data.depth_settings.r, 10.0f, "%.1f");
                    if (ImGui::IsItemHovered())
                    {
                        ImGuiB3D::ToolTipExtendedText("Set the depth testing far distance with white color.\nThis determines how far the pixel is to the viewpoint.", TOOL_TIP_WIDTH);
                    }

                    ImGui::PopItemWidth();

                    if (is_disabled) 
                    { 
                        ImGui::EndDisabled(); 
                    }
                }
            }

            ImGui::EndTable();
        }

        // Apply changes back to global states
        GlobalSettings::set_global_setting<int>(GlobalSettingOption::VisualMode, view_selection);
        pages_data.debug_mode = view_selection;
        Renderer::set_pages_data(pages_data);

        ImGui::EndPopup();
    }
}


void Viewport::draw_editor_overlays_selection()
{
    if (ImGui::Button("Editor Overlays"))
    {
        ImGui::OpenPopup("Editor Overlays Popup");
    }

    if (ImGui::BeginPopup("Editor Overlays Popup"))
    {
        ImGui::SeparatorText("Editor Overlays");

        for (auto& [flag, label] : overlay_flags_list)
        {
            const bool is_selected = (current_overlay_flags & flag) != 0;
            ImGui::PushItemFlag(ImGuiItemFlags_AutoClosePopups, false);
            const bool clicked = ImGui::MenuItem(label, nullptr, is_selected);
            ImGui::PopItemFlag();

            if (clicked)
            {
                current_overlay_flags ^= flag;
            }
        }

        ImGui::EndPopup();
    }

    uint32_t overlay_flags_numbered = static_cast<uint32_t>(current_overlay_flags);
    if (GlobalSettings::get_global_setting_value<uint32_t>(GlobalSettingOption::DebugGeometry_Enabled) != overlay_flags_numbered)
    {
        GlobalSettings::set_global_setting<uint32_t>(GlobalSettingOption::DebugGeometry_Enabled, overlay_flags_numbered);
    }
}
