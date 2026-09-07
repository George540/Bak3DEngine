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

#include "splash_screen.h"

#include "imgui_b3d_extensions.h"
#include "Asset/resource_manager.h"

constexpr static float IMGUI_SPLASH_SCREEN_PADDING_HORIZONTAL = 10.0f;

SplashScreen::SplashScreen() : EditorPanel("Splash Screen")
{
    ImGui::SetWindowFocus(m_title);
    m_size = ImVec2(500, 600);
    m_flags |= ImGuiWindowFlags_NoMove
                | ImGuiWindowFlags_NoTitleBar
                | ImGuiWindowFlags_NoResize
                | ImGuiWindowFlags_NoDocking
                | ImGuiWindowFlags_NoScrollbar;
}

void SplashScreen::begin_frame()
{
    if (!m_visible)
    {
        return;
    }
    
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 center_pos = ImVec2(
        viewport->WorkPos.x + viewport->WorkSize.x * 0.5f,
        viewport->WorkPos.y + viewport->WorkSize.y * 0.5f
    );
    ImGui::SetNextWindowPos(center_pos, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(m_size);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin(m_title, nullptr, m_flags);
}

void SplashScreen::update()
{
    if (!m_visible)
    {
        return;
    }

    draw_splash_image();

    ImGuiB3D::SeparatorWithSpacing(2);

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + IMGUI_SPLASH_SCREEN_PADDING_HORIZONTAL);

    ImGui::BeginGroup();

    draw_intro_text();

    ImGuiB3D::MultiSpacing(4);

    draw_links_column_left();

    ImGui::EndGroup();
}

void SplashScreen::end_frame()
{
    if (!m_visible)
    {
        return;
    }

    bool clicked_away = false;
    if (!ImGui::IsWindowAppearing())
    {
        if (!ImGui::IsWindowAppearing())
        {
            clicked_away = ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
        }
    }

    ImGui::PopStyleVar();
    EditorPanel::end_frame();

    if (clicked_away)
    {
        m_visible = false;
        ImGui::SetWindowFocus("Viewport");
    }
}

void SplashScreen::draw_splash_image()
{
    const TextureRef texture_ref = ResourceManager::get_texture("splash_screen_image.png");
    const ImTextureID splash_screen_image_id = texture_ref->get_texture_id();
    const ImVec2 texture_size = ImVec2(texture_ref->get_width(), texture_ref->get_height());

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    ImGui::Image(splash_screen_image_id, texture_size, ImVec2(0, 1), ImVec2(1, 0));
    ImGui::PopStyleVar();
}

void SplashScreen::draw_intro_text()
{
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + (m_size.x - (IMGUI_SPLASH_SCREEN_PADDING_HORIZONTAL * 4.0f)));
    ImGui::TextWrapped("Welcome to Bak3D Engine! This is a beginner-friendly 3D graphics (and soon gaming) engine that gradually implements different rendering and engine related technical concepts. "
                           "This engine is open for any level of difficulties and it is not restricted to advanced-only individuals. Everyone is welcome to explore, contribute, clone and repurpose, "
                           "as long as attribution is included.");
    ImGui::PopTextWrapPos();

    ImGuiB3D::MultiSpacing(1);
    
    ImGui::TextWrapped("Here are some interesting links and things to get started with before hopping in the engine:");
}

void SplashScreen::draw_links_column_left()
{
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (2.0f * IMGUI_SPLASH_SCREEN_PADDING_HORIZONTAL));

    ImGui::BeginGroup();
    
    ImGui::TextLinkOpenURL("GitHub Repository", "https://github.com/George540/Bak3DEngine");
    ImGui::Spacing();
    ImGui::TextLinkOpenURL("Contributing", "https://github.com/George540/Bak3DEngine");
    ImGui::Spacing();
    ImGui::TextLinkOpenURL("Copyright License", "https://opensource.org/license/mit");
    ImGui::Spacing();
    ImGui::TextUnformatted("Discord: generalmoai");

    ImGui::EndGroup();
}
