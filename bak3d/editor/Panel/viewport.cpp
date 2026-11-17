#include "viewport.h"

#include <imgui_internal.h>
#include <implot.h>
#include <glm/common.hpp>

#include "imgui_b3d_extensions.h"
#include "Asset/resource_manager.h"
#include "Core/global_settings.h"
#include "Input/event_manager.h"
#include "Input/renderdoc_manager.h"
#include "Renderer/light_renderer.h"
#include "Renderer/post_processor.h"
#include "Renderer/renderer.h"
#include "Renderer/shadow_renderer.h"
#include "Scene/scene_manager.h"

using namespace std;

namespace
{
    constexpr float IMGUI_VIEWPORT_POPUP_WIDTH = 280.0f;
    constexpr float IMGUI_VIEWPORT_POPUP_ENTRY_HEIGHT = 25.0f;
    constexpr float IMGUI_VIEWPORT_HIGHLIGHT_POPUP_HORIZONTAL = 256.0f;
    constexpr float IMGUI_PREVIEW_OVERLAY_MARGIN = 10.0f;

    ImVec2 previous_viewport_size = ImVec2(0, 0);
    ImVec2 viewport_panel_size = ImVec2(0, 0);
    bool m_camera_look_started_in_viewport = false;
    ImVec2 viewport_image_min = ImVec2(0, 0);
    ImVec2 viewport_image_max = ImVec2(0, 0);

    OverlaysFlags current_overlay_flags = OverlaysFlags::None;

    vector<OverlayFlagsDefinition> overlay_flags_list;

    float popup_opacity_value = 0.5f;

    void popup_opacity_slider()
    {
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::SliderFloat("##popup_opacity_value", &popup_opacity_value, 0.0f, 1.0f);
    }

    void push_popup_style()
    {
        const ImVec2 button_position = ImGui::GetItemRectMin();
        const float button_size_vertical = ImGui::GetItemRectSize().y;

        ImGui::SetNextWindowSize(ImVec2(IMGUI_VIEWPORT_POPUP_WIDTH, 0.0f));
        ImGui::SetNextWindowPos(ImVec2(button_position.x, button_position.y + button_size_vertical));
        ImVec4 popup_bg_color = ImGui::GetStyle().Colors[ImGuiCol_PopupBg];
        popup_bg_color.w = popup_opacity_value;
        ImGui::PushStyleColor(ImGuiCol_PopupBg, popup_bg_color);
    }

    string format_count(const uint64_t value)
    {
        string value_string = to_string(value);
        for (int i = static_cast<int>(value_string.size()) - 3; i > 0; i -= 3)
        {
            value_string.insert(i, ",");
        }
        return value_string;
    }

    bool begin_stats_section(const char* title, const char* table_id)
    {
        ImGui::SeparatorText(title);
        if (!ImGui::BeginTable(table_id, 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoSavedSettings))
        {
            return false;
        }
        ImGui::TableSetupColumn("Stat Name", ImGuiTableColumnFlags_WidthStretch, 0.6f);
        ImGui::TableSetupColumn("Stat Value", ImGuiTableColumnFlags_WidthStretch, 0.4f);
        return true;
    }

    void stat_row(const char* name, const string& value, const char* tooltip = nullptr)
    {
        ImGui::TableNextRow(ImGuiTableRowFlags_None, IMGUI_VIEWPORT_POPUP_ENTRY_HEIGHT);
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(name);
        if (tooltip && ImGui::IsItemHovered())
        {
            ImGuiB3D::ToolTipExtendedText(tooltip);
        }
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(value.c_str());
    }

    void begin_tool_row(const char* name, const char* tooltip = nullptr)
    {
        ImGui::TableNextRow(ImGuiTableRowFlags_None, IMGUI_VIEWPORT_POPUP_ENTRY_HEIGHT);
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(name);
        if (tooltip && ImGui::IsItemHovered())
        {
            ImGuiB3D::ToolTipExtendedText(tooltip);
        }
        ImGui::TableSetColumnIndex(1);
    }
}

Viewport::Viewport() : EditorPanel("Viewport")
{
    m_flags |= ImGuiWindowFlags_NoScrollbar;

    current_overlay_flags = static_cast<OverlaysFlags>(GlobalSettings::get_global_setting_value<uint32_t>(GlobalSettingOption::EditorOverlayFlags));

    overlay_flags_list =
    {
        {.flag = OverlaysFlags::WorldGrid, .label = "Grid", .description = "Show the 10x10 XZ world grid in origin." },
        {.flag = OverlaysFlags::LightIcons, .label = "Light Icons", .description = "Show editor-only icons at light positions." },
        {.flag = OverlaysFlags::LightGeometry, .label = "Light Geometry", .description = "Show editor-only debug shapes for lights in the scene." },
        {.flag = OverlaysFlags::ObjectPreview, .label = "Object Preview", .description = "Show a debug preview for objects that have a debug preview (ex: camera, lights)." },
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
    draw_preview_overlay();
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
    const auto view_mode = static_cast<DebugViewMode>(GlobalSettings::get_global_setting_value<int>(GlobalSettingOption::ViewMode));
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

    viewport_image_min = ImGui::GetItemRectMin();
    viewport_image_max = ImGui::GetItemRectMax();

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

void Viewport::draw_preview_overlay()
{
    if (!(GlobalSettings::get_global_setting_value<uint32_t>(GlobalSettingOption::EditorOverlayFlags) & static_cast<uint32_t>(OverlaysFlags::ObjectPreview)))
    {
        return;
    }

    const Scene* scene = SceneManager::get_current_scene();
    const SceneObject* selected = scene ? scene->get_selected_scene_object() : nullptr;
    if (!selected)
    {
        return;
    }

    GLuint preview_texture = 0;
    ImVec2 image_size;

    if (selected->get_object_type() == SceneObjectType::Camera)
    {
        if (selected == scene->get_current_camera()) // @TODO: Make sure that it only applies to game scene cameras and not editor cameras
        {
            const FrameBuffer* main_fbo = Renderer::get_main_frame_buffer();
            preview_texture = main_fbo->get_color_texture();
            image_size = ImVec2(IMGUI_VIEWPORT_HIGHLIGHT_POPUP_HORIZONTAL, IMGUI_VIEWPORT_HIGHLIGHT_POPUP_HORIZONTAL / main_fbo->get_aspect_ratio());
        }
    }
    else if (selected->get_object_type() == SceneObjectType::Light)
    {
        preview_texture = ShadowRenderer::get_light_preview_texture(dynamic_cast<const Light*>(selected));
        image_size = ImVec2(IMGUI_VIEWPORT_HIGHLIGHT_POPUP_HORIZONTAL, IMGUI_VIEWPORT_HIGHLIGHT_POPUP_HORIZONTAL);

        if (preview_texture == 0)
        {
            return; // Early exist for non-shadow casters
        }
    }
    else
    {
        return;
    }

    const ImGuiStyle& style = ImGui::GetStyle();
    const ImVec2 panel_size(image_size.x + style.WindowPadding.x * 2.0f,
                            image_size.y + ImGui::GetTextLineHeight() + style.ItemSpacing.y + style.WindowPadding.y * 2.0f);

    // Skip if the viewport is too small to host the popup
    const ImVec2 viewport_size(viewport_image_max.x - viewport_image_min.x, viewport_image_max.y - viewport_image_min.y);
    if (viewport_size.x < panel_size.x + IMGUI_PREVIEW_OVERLAY_MARGIN * 2.0f || viewport_size.y < panel_size.y + IMGUI_PREVIEW_OVERLAY_MARGIN * 2.0f)
    {
        return;
    }

    ImGui::SetCursorScreenPos(ImVec2(viewport_image_min.x + IMGUI_PREVIEW_OVERLAY_MARGIN, viewport_image_max.y - panel_size.y - IMGUI_PREVIEW_OVERLAY_MARGIN));

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.10f, 0.10f, 0.10f, 0.75f));
    if (ImGui::BeginChild("##Object_Preview_Overlay", panel_size, ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
        ImGui::TextUnformatted(ImGuiB3D::TruncateLabel(selected->get_object_name(), image_size.x).c_str());

        if (preview_texture != 0)
        {
            ImGui::Image(preview_texture, image_size, ImVec2(0, 1), ImVec2(1, 0));
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
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
    draw_visual_modes_popup();

    ImGui::SameLine();

    draw_editor_overlays_popup();

    ImGui::SameLine();

    draw_stats_popup();

    ImGui::SameLine();

    draw_gpu_tools_popup();

    ImGui::SameLine();
}

void Viewport::draw_visual_modes_popup()
{
    if (ImGui::Button("View Modes"))
    {
        ImGui::OpenPopup("View Modes Popup");
    }

    push_popup_style();
    if (ImGui::BeginPopup("View Modes Popup", ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove))
    {
        popup_opacity_slider();

        ImGui::SeparatorText("View Modes");

        int view_selection = GlobalSettings::get_global_setting_value<int>(GlobalSettingOption::ViewMode);
        PagesData pages_data = Renderer::get_pages_data();

        // Initialize Table
        if (ImGui::BeginTable("ViewModesTable", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoSavedSettings))
        {
            // Setup columns: Left holds selection, Right holds settings
            ImGui::TableSetupColumn("Selection", ImGuiTableColumnFlags_WidthStretch, 0.4f);
            ImGui::TableSetupColumn("Settings", ImGuiTableColumnFlags_WidthStretch, 0.6f);

            for (int view_mode_num = 0; view_mode_num < static_cast<int>(DebugViewMode::Count); ++view_mode_num)
            {
                const auto view_mode = static_cast<DebugViewMode>(view_mode_num);
                ImGui::TableNextRow(ImGuiTableRowFlags_None, IMGUI_VIEWPORT_POPUP_ENTRY_HEIGHT);
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

                    ImGui::PushItemWidth(60.0f);

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
                    ImGui::Text("Far ");
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
        GlobalSettings::set_global_setting<int>(GlobalSettingOption::ViewMode, view_selection);
        pages_data.debug_mode = view_selection;
        Renderer::set_pages_data(pages_data);

        ImGui::EndPopup();
    }
    ImGui::PopStyleColor();
}


void Viewport::draw_editor_overlays_popup()
{
    if (ImGui::Button("Editor Overlays"))
    {
        ImGui::OpenPopup("Editor Overlays Popup");
    }

    push_popup_style();
    if (ImGui::BeginPopup("Editor Overlays Popup", ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove))
    {
        popup_opacity_slider();

        ImGui::SeparatorText("Editor Overlays");

        float light_icon_size = GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::EditorOverlayFlags_LightIconSize);

        // Initialize Table
        if (ImGui::BeginTable("EditorOverlaysTable", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoSavedSettings))
        {
            // Setup columns: Left holds selection, Right holds settings
            ImGui::TableSetupColumn("Selection", ImGuiTableColumnFlags_WidthStretch, 0.5f);
            ImGui::TableSetupColumn("Settings", ImGuiTableColumnFlags_WidthStretch, 0.5f);

            for (auto& [flag, label, description] : overlay_flags_list)
            {
                ImGui::TableNextRow(ImGuiTableRowFlags_None, IMGUI_VIEWPORT_POPUP_ENTRY_HEIGHT);
                ImGui::TableSetColumnIndex(0);

                const bool is_selected = (current_overlay_flags & flag) != 0;
                ImGui::PushItemFlag(ImGuiItemFlags_AutoClosePopups, false);
                const bool clicked = ImGui::MenuItem(label, nullptr, is_selected);
                ImGui::PopItemFlag();

                if (description)
                {
                    ImGuiB3D::ToolTipExtendedText(description, TOOL_TIP_WIDTH);
                }

                if (clicked)
                {
                    current_overlay_flags ^= flag;
                }

                ImGui::TableSetColumnIndex(1);

                if (flag == OverlaysFlags::LightIcons)
                {
                    const bool is_disabled = !((current_overlay_flags & flag) != 0);
                    if (is_disabled)
                    {
                        ImGui::BeginDisabled();
                    }

                    ImGui::PushItemWidth(100.0f);
                    ImGui::SliderFloat("##LightIconSize", &light_icon_size, 0.05f, 0.3f, "%.2f");
                    if (ImGui::IsItemHovered())
                    {
                        ImGuiB3D::ToolTipExtendedText("Control the size of light icon sprites drawn in the viewport.", TOOL_TIP_WIDTH);
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

        GlobalSettings::set_global_setting<float>(GlobalSettingOption::EditorOverlayFlags_LightIconSize, light_icon_size);

        ImGui::EndPopup();
    }

    uint32_t overlay_flags_numbered = static_cast<uint32_t>(current_overlay_flags);
    if (GlobalSettings::get_global_setting_value<uint32_t>(GlobalSettingOption::EditorOverlayFlags) != overlay_flags_numbered)
    {
        GlobalSettings::set_global_setting<uint32_t>(GlobalSettingOption::EditorOverlayFlags, overlay_flags_numbered);
    }

    ImGui::PopStyleColor();
}

void Viewport::draw_stats_popup()
{
    if (ImGui::Button("Scene Stats"))
    {
        ImGui::OpenPopup("Scene Stats Popup");
    }

    push_popup_style();
    if (ImGui::BeginPopup("Scene Stats Popup", ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove))
    {
        popup_opacity_slider();

        const SceneStats& scene_stats = SceneManager::get_current_scene()->gather_scene_stats();

        if (begin_stats_section("Lights", "LightsStatsTable"))
        {
            stat_row("Total", format_count(scene_stats.lights_total), "All lights registered in the scene.");
            stat_row("Active", format_count(scene_stats.lights_active), "All active light objects.");
            stat_row("Visible", format_count(LightRenderer::get_visible_light_count()), "Lights sent to the GPU.");
            ImGui::EndTable();
        }

        if (begin_stats_section("Meshes", "MeshesStatsTable"))
        {
            stat_row("Total", format_count(scene_stats.meshes_total), "All meshes registered in the scene.");
            stat_row("Active", format_count(scene_stats.meshes_active), "All active mesh objects.");
            stat_row("Visible", format_count(scene_stats.meshes_visible), "All active and visible meshes in the camera's view.");
            ImGui::EndTable();
        }

        if (begin_stats_section("Particles", "ParticlesStatsTable"))
        {
            stat_row("Total Systems", format_count(scene_stats.particle_systems_total), "All particle systems registered in the scene.");
            stat_row("Active Systems", format_count(scene_stats.particle_systems_active), "All active particle system objects.");
            stat_row("Visible Systems", format_count(scene_stats.particle_systems_visible), "All active and visible particle systems in the camera's view,\ndetermined through their bounding box.");

            stat_row("Total Emitters", format_count(scene_stats.particle_systems_emitters_total), "All particle emitters registered in the scene.");
            stat_row("Active Emitters", format_count(scene_stats.particle_systems_emitters_active), "All active particle emitter objects.");
            stat_row("Visible Emitters", format_count(scene_stats.particle_systems_emitters_visible), "All active and visible particle emitter objects.");
            
            stat_row("Total Particles", format_count(scene_stats.particle_systems_particles_total), "All particles in the scene.");

            ImGui::EndTable();
        }

        ImGui::EndPopup();
    }

    ImGui::PopStyleColor();
}

void Viewport::draw_gpu_tools_popup()
{
    if (ImGui::Button("GPU Debugging"))
    {
        ImGui::OpenPopup("GPU Debugging Popup");
    }

    push_popup_style();
    if (ImGui::BeginPopup("GPU Debugging Popup", ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove))
    {
        popup_opacity_slider();

        if (begin_stats_section("Shaders", "ShadersTable"))
        {
            begin_tool_row("Hot Reload", "Hot-Reload shaders after updating shader files after running sync_shaders.py script");
            if (ImGui::Button("Reload"))
            {
                ResourceManager::reload_shaders();
            }

            bool force_fail = GlobalSettings::get_global_setting_value<bool>(GlobalSettingOption::Resources_ForceFail);
            begin_tool_row("Force Fail", "Force shaders to output visual results even if they fail during compilation or hot-reload");
            ImGui::Checkbox("##ForceFail", &force_fail);
            GlobalSettings::set_global_setting<bool>(GlobalSettingOption::Resources_ForceFail, force_fail);
            
            ImGui::EndTable();
        }

        if (begin_stats_section("Tools", "GPUToolsTable"))
        {
            begin_tool_row("RenderDoc Capture", "Trigger GPU frame capture if RenderDoc app is available. Make sure it's installed locally first!");
            if (ImGui::Button("Capture Frame"))
            {
                RenderDocManager::trigger_capture();
            }

            bool vsync = GlobalSettings::get_global_setting_value<bool>(GlobalSettingOption::Vsync);
            begin_tool_row("VSync", "Toggle VSync. If enabled, GLFW frame rate synchronizes with display's refresh rate");
            if (ImGui::Checkbox("##VSync", &vsync))
            {
                EventManager::toggle_vsync(vsync);
                GlobalSettings::set_global_setting<bool>(GlobalSettingOption::Vsync, vsync);
            }
            
            ImGui::EndTable();
        }

        ImGui::EndPopup();
    }

    ImGui::PopStyleColor();
}
