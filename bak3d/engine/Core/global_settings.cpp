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

#include "global_settings.h"

std::map<GlobalSettingOption, GlobalSettingValueType> GlobalSettings::global_settings = {};

// TODO: to be used for property labels in ImGui
const char* GlobalSettings::to_string(GlobalSettingOption enum_setting)
{
    switch (enum_setting)
    {
        case GlobalSettingOption::Resources_ForceFail: return "Force Fail";
        case GlobalSettingOption::Vsync: return "VSync";
        case GlobalSettingOption::DebugGeometry_Enabled: return "Enable Debug Geometry";
        case GlobalSettingOption::VisualMode: return "Visual Mode";
        case GlobalSettingOption::BackgroundColor: return "Background Color";
        case GlobalSettingOption::AA_MSAA_Enabled: return "MSAA Enabled";
        case GlobalSettingOption::AA_MSAA_Samples: return "MSAA Samples";
        case GlobalSettingOption::PostProcessing_Enabled: return "Enabled";
        case GlobalSettingOption::PostProcess_ColorGrading_Invert: return "Invert";
        case GlobalSettingOption::PostProcess_ColorGrading_Brightness: return "Brightness";
        case GlobalSettingOption::PostProcess_ColorGrading_Contrast: return "Contrast";
        case GlobalSettingOption::PostProcess_ColorGrading_Grayscale: return "Grayscale";
        case GlobalSettingOption::PostProcess_ColorGrading_Hue: return "Hue";
        case GlobalSettingOption::PostProcess_ColorGrading_Saturation: return "Saturation";
        case GlobalSettingOption::PostProcess_ColorGrading_Temperature: return "Temperature";
        case GlobalSettingOption::PostProcess_KernelEffect_SharpenIntensity: return "Sharpen";
        case GlobalSettingOption::PostProcess_KernelEffect_SobelIntensity: return "Sobel";
        case GlobalSettingOption::PostProcess_KernelEffect_EmbossIntensity: return "Emboss";
        case GlobalSettingOption::PostProcess_KernelEffect_BoxBlurIntensity: return "Box Blur";
        case GlobalSettingOption::PostProcess_KernelEffect_LaplacianIntensity: return "Laplacian";
        case GlobalSettingOption::Max: return "Max";
        default: return "Unknown";
    }
}

void GlobalSettings::initialize()
{
    global_settings.clear();
    global_settings[GlobalSettingOption::Resources_ForceFail] = false;
    global_settings[GlobalSettingOption::Vsync] = false;
    global_settings[GlobalSettingOption::DebugGeometry_Enabled] = static_cast<uint32_t>(OverlaysFlags::All);
    global_settings[GlobalSettingOption::VisualMode] = 0;
    global_settings[GlobalSettingOption::BackgroundColor] = glm::vec4(0.133f, 0.168f, 0.2f, 1.0f);
    global_settings[GlobalSettingOption::AA_MSAA_Enabled] = true;
    global_settings[GlobalSettingOption::AA_MSAA_Samples] = 4; // turns to 4x4 when passed to GLFW
    global_settings[GlobalSettingOption::PostProcessing_Enabled] = false;
    global_settings[GlobalSettingOption::PostProcess_ColorGrading_Invert] = false;
    global_settings[GlobalSettingOption::PostProcess_ColorGrading_Grayscale] = false;
    global_settings[GlobalSettingOption::PostProcess_ColorGrading_Brightness] = 0.0f;
    global_settings[GlobalSettingOption::PostProcess_ColorGrading_Contrast] = 0.0f;
    global_settings[GlobalSettingOption::PostProcess_ColorGrading_Hue] = 0.0f;
    global_settings[GlobalSettingOption::PostProcess_ColorGrading_Saturation] = 0.0f;
    global_settings[GlobalSettingOption::PostProcess_ColorGrading_Temperature] = 0.0f;
    global_settings[GlobalSettingOption::PostProcess_ColorGrading_VignetteIntensity] = 0.0f;
    global_settings[GlobalSettingOption::PostProcess_ColorGrading_VignetteColor] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    global_settings[GlobalSettingOption::PostProcess_KernelEffect_SharpenIntensity] = 0.0f;
    global_settings[GlobalSettingOption::PostProcess_KernelEffect_SobelIntensity] = 0.0f;
    global_settings[GlobalSettingOption::PostProcess_KernelEffect_EmbossIntensity] = 0.0f;
    global_settings[GlobalSettingOption::PostProcess_KernelEffect_BoxBlurIntensity] = 0.0f;
    global_settings[GlobalSettingOption::PostProcess_KernelEffect_LaplacianIntensity] = 0.0f;
}

void GlobalSettings::shutdown()
{
    global_settings.clear();
}

void GlobalSettings::reset_to_defaults()
{
    initialize();
}
