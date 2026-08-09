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
        case GlobalSettingOption::GridRendering: return "Enable Grid";
        case GlobalSettingOption::AxisRendering: return "Enable Axis";
        case GlobalSettingOption::BackgroundColor: return "Background Color";
        case GlobalSettingOption::Light_Enabled: return "Enabled";
        case GlobalSettingOption::Light_Type: return "Type";
        case GlobalSettingOption::Light_HorizontalRotation: return "Horizontal Rotation";
        case GlobalSettingOption::Light_VerticalRotation: return "Vertical Rotation";
        case GlobalSettingOption::Light_Scaling: return "Scaling";
        case GlobalSettingOption::Light_OriginDistance: return "Origin Distance";
        case GlobalSettingOption::Light_Color: return "Color";
        case GlobalSettingOption::Light_Intensity: return "Intensity";
        case GlobalSettingOption::Light_Point_Attenuation_Radius: return "Attenuation Radius";
        case GlobalSettingOption::Light_Spot_ConeAngle_Size: return "Cone Size";
        case GlobalSettingOption::Light_Spot_ConeAngle_Inner_CutOff: return "Inner Cutoff";
        case GlobalSettingOption::Light_Spot_ConeAngle_Outer_CutOff: return "Outer Cutoff";
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
        case GlobalSettingOption::DebugView_DepthTesting: return "View Depth Testing";
        case GlobalSettingOption::Max: return "Max";
        default: return "Unknown";
    }
}

void GlobalSettings::initialize()
{
    global_settings.clear();
    global_settings[GlobalSettingOption::Resources_ForceFail] = false;
    global_settings[GlobalSettingOption::Vsync] = false;
    global_settings[GlobalSettingOption::GridRendering] = true;
    global_settings[GlobalSettingOption::AxisRendering] = true;
    global_settings[GlobalSettingOption::BackgroundColor] = glm::vec4(0.133f, 0.168f, 0.2f, 1.0f);
    global_settings[GlobalSettingOption::Light_Enabled] = true;
    global_settings[GlobalSettingOption::Light_Type] = static_cast<uint32_t>(1);
    global_settings[GlobalSettingOption::Light_HorizontalRotation] = 45.0f;
    global_settings[GlobalSettingOption::Light_VerticalRotation] = 135.0f;
    global_settings[GlobalSettingOption::Light_Scaling] = 0.5f;
    global_settings[GlobalSettingOption::Light_OriginDistance] = 5.0f;
    global_settings[GlobalSettingOption::Light_Color] = glm::vec4(1.0f);
    global_settings[GlobalSettingOption::Light_Intensity] = 1.0f;
    global_settings[GlobalSettingOption::Light_Point_Attenuation_Radius] = 100.0f;
    global_settings[GlobalSettingOption::Light_Spot_ConeAngle_Size] = 1.0f;
    global_settings[GlobalSettingOption::Light_Spot_ConeAngle_Inner_CutOff] = 12.5f;
    global_settings[GlobalSettingOption::Light_Spot_ConeAngle_Outer_CutOff] = 17.5f;
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
    global_settings[GlobalSettingOption::DebugView_DepthTesting] = true;
}

void GlobalSettings::shutdown()
{
    global_settings.clear();
}

void GlobalSettings::reset_to_defaults()
{
    initialize();
}
