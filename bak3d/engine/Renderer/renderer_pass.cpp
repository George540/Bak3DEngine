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

#include "debug_scope.h"
#include "post_processor.h"
#include "renderer.h"
#include "Asset/resource_manager.h"
#include "Core/global_settings.h"
#include "Scene/scene.h"

void RendererPasses::render_pass_debug_geometry()
{
    DebugScopeGroup scope("Debug Geometry Pass");

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
    DebugScopeGroup scope("Base Geometry Pass");

    if (const Model* model = Scene::instance->get_model())
    {
        model->draw();
    }
    if (const ParticleSystem* particle_system = Scene::instance->get_particle_system())
    {
        particle_system->draw();
    }
}

void RendererPasses::render_pass_lighting()
{
    DebugScopeGroup scope("Lighting (Geometry) Pass");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    if (GlobalSettings::get_global_setting_value<bool>(GlobalSettingOption::Light_Enabled))
    {
        Scene::instance->get_active_light()->draw();
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void RendererPasses::render_pass_transparency()
{
    DebugScopeGroup scope("Transparency Pass (WBOIT)");

    AdvancedParticleSystem* advanced_particles = Scene::instance->get_advanced_particle_system();
    if (!advanced_particles)
    {
        return;
    }

    const WBOITFrameBuffer* wboit_fbo = Renderer::get_wboit_frame_buffer();
    
    // Accumulate: particles write into accum + revealage targets
    {
        DebugScopeGroup accumulate_scope("WBOIT Accumulate");

        wboit_fbo->bind();
        wboit_fbo->clear();

        glEnable(GL_DEPTH_TEST); // still occluded by opaque geometry
        glDepthMask(GL_FALSE); // never write depth on order-independene pass

        glEnable(GL_BLEND);
        glBlendFunci(0, GL_ONE, GL_ONE); // accum target: additive
        glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR); // revealage target: product of (1 - alpha)
        glBlendEquation(GL_FUNC_ADD);

        advanced_particles->draw();

        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        glDisable(GL_DEPTH_TEST);

        wboit_fbo->unbind();
    }

    // Composite: merge accumulate/revealage back onto the resolved opaque frame
    {
        DebugScopeGroup composite_scope("WBOIT Composite");

        const ShaderRef composite_shader = ResourceManager::get_shader("wboit_composite");
        if (!composite_shader || !composite_shader->is_shader_compiled())
        {
            return;
        }

        const FrameBuffer* opaque_fbo = Renderer::get_main_frame_buffer();
        opaque_fbo->bind();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        composite_shader->use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, wboit_fbo->get_accum_texture());
        composite_shader->set_int("accumulation_texture", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, wboit_fbo->get_revealage_texture());
        composite_shader->set_int("revealage_texture", 1);

        Renderer::draw_quad();

        composite_shader->unuse();

        glDisable(GL_BLEND);

        opaque_fbo->unbind();
    }
}

void RendererPasses::render_pass_post_processing()
{
    DebugScopeGroup scope("Post Processing Pass");

    if (GlobalSettings::get_global_setting_value<bool>(GlobalSettingOption::PostProcessing_Enabled))
    {
        PostProcessor::process_frame(*Renderer::get_main_frame_buffer());
    }
}

void RendererPasses::render_pass_debug_view()
{
    DebugScopeGroup scope("Debug View Pass");

    const ShaderRef debug_view_shader = ResourceManager::get_shader("debug_view");

    if (!debug_view_shader || !debug_view_shader->is_shader_compiled())
    {
        return;
    }

    Renderer::get_debug_view_buffer()->bind();

    debug_view_shader->use();

    const int depth_texture = static_cast<int>(Renderer::get_main_frame_buffer()->get_depth_texture());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depth_texture);
    debug_view_shader->set_int("depth_texture", 0);

    Renderer::draw_quad();

    debug_view_shader->unuse();

    Renderer::get_debug_view_buffer()->unbind();
}
