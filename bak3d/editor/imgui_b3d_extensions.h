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

#pragma once

#include <imgui.h>
#include <string>
#include <vector>

class ImGuiB3D
{
public:
    static bool PropertyToggle(const char* label, bool* value, const char* tooltip_desc = nullptr);
    static bool PropertyColorPicker(const char* label, float* color, const char* tooltip_desc = nullptr);
    static bool PropertyDropdown(const char* label, const std::vector<const char*>& data, int* selected_index, const char* tooltip_desc = nullptr);
    static bool PropertyBeginDropdown(const char* label, const char* preview_value, const char* tooltip_desc);
    static bool PropertySliderFloat(const char* label, float* value, float v_min, float v_max, const char* format, const char* tooltip_desc = nullptr);
    static bool PropertyButton(const char* property_label, const char* button_label = nullptr, const char* tooltip_desc = nullptr, ImVec2 size = ImVec2(0, 0));
    static bool PropertyImageButton(const char* label, const char* tooltip_desc, ImTextureID texture_id, ImVec2 size, ImVec2 uv0 = ImVec2( 1.0f, 1.0f), ImVec2 uv1 = ImVec2( 0.0f, 0.0f), ImVec4 bg_col = ImVec4(0.0f, 0.0f, 0.0f, 0.0f), ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    
    static bool SeparatorWithSpacing(int num_spaces);
    static bool StringContainsIgnoreCase(std::string str, const std::string& sub_str);
private:
    static bool ToolTipExtendedText(const char* tooltip_desc, float text_wrap_size);
    static bool ToolTipExtendedAsset(const char* tooltip_desc, float text_wrap_size);
    static bool MultiSpacing(int num_spaces);
};
