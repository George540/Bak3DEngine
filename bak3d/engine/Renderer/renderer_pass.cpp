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

#include "renderer_pass.h"

#include "post_processor.h"
#include "renderer.h"
#include "Core/global_settings.h"
#include "Scene/scene.h"

void RendererPasses::render_pass_debug_geometry()
{
    const bool is_grid_rendering = GlobalSettings::get_global_setting_value<bool>(GlobalSettingOption::GridRendering);
    const bool is_axis_rendering = GlobalSettings::get_global_setting_value<bool>(GlobalSettingOption::AxisRendering);
    if (is_grid_rendering)
    {
        Scene::instance->get_object_in_scene(SceneObjectType::Grid)->draw();
    }
    if (is_axis_rendering)
    {
        Scene::instance->get_object_in_scene(SceneObjectType::Axis)->draw();
    }
}

void RendererPasses::render_pass_base_geometry()
{
    if (const Model* model = Scene::instance->get_model())
    {
        model->draw();
    }
    if (const ParticleSystem* particle_system = Scene::instance->get_particle_system())
    {
        particle_system->draw();
    }
    if (const AdvancedParticleSystem* advanced_particle_system = Scene::instance->get_advanced_particle_system())
    {
        advanced_particle_system->draw();
    }
}

void RendererPasses::render_pass_lighting()
{
    if (GlobalSettings::get_global_setting_value<bool>(GlobalSettingOption::Light_Enabled))
    {
        Scene::instance->get_active_light()->draw();
    }
}

void RendererPasses::render_pass_post_processing()
{
    if (GlobalSettings::get_global_setting_value<bool>(GlobalSettingOption::PostProcessing_Enabled))
    {
        PostProcessor::process_frame(*Renderer::get_frame_buffer());
    }
}
