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

    glDepthFunc(GL_ALWAYS);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE); 

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

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); 
    glDepthFunc(GL_LESS);
}

void RendererPasses::render_pass_opaque_geometry()
{
    DebugScopeGroup scope("Opaque Geometry Pass");

    if (const Model* model = Scene::instance->get_model())
    {
        model->draw();
    }
}

void RendererPasses::render_pass_sprites()
{
    DebugScopeGroup scope("Sprites Pass");

    const ParticleSystem* particle_system = Scene::instance->get_particle_system();
    if (!particle_system)
    {
        return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE); 

    particle_system->draw();

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); 
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
}

void RendererPasses::render_pass_transparency()
{
    DebugScopeGroup scope("Transparency Pass (WBOIT)");

    AdvancedParticleSystem* advanced_particles = Scene::instance->get_advanced_particle_system();
    if (!advanced_particles)
    {
        return;
    }

    WBOITFrameBuffer* wboit_fbo = Renderer::get_wboit_frame_buffer();
    FrameBuffer* resolved_base_fbo = Renderer::get_main_frame_buffer();

    if (!wboit_fbo || !resolved_base_fbo)
    {
        return;
    }

    // wboit_fbo shares r_main_fbo's depth-stencil TEXTURE object directly.
    // No copy/resolve needed for opaque occlusion to already be visible here.

    // Accumulate: particles write into accum + revealage, depth-tested
    // against (but not writing) the already-resolved opaque depth.
    {
        DebugScopeGroup accumulate_scope("WBOIT Accumulate");

        wboit_fbo->bind();
        wboit_fbo->clear();

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);

        glBlendFunci(0, GL_ONE, GL_ONE);                  // accumulation target: additive
        glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR); // revealage target: product of (1 - alpha)
        glBlendEquation(GL_FUNC_ADD);

        advanced_particles->draw();

        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);

        wboit_fbo->unbind();
    }

    // Composite: blend accum/revealage onto the resolved opaque frame via a
    // full-screen quad (shader read, not a blit). Deliberately does NOT
    // unbind resolved_base_fbo - render_pass_editor_overlays() draws into it next.
    {
        DebugScopeGroup composite_scope("WBOIT Composite");

        const ShaderRef composite_shader = ResourceManager::get_shader("wboit_composite");
        if (!composite_shader || !composite_shader->is_shader_compiled())
        {
            return;
        }

        resolved_base_fbo->bind();

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);

        composite_shader->use();

        wboit_fbo->bind_color_attachment(0); // accumulation_texture -> unit 0
        composite_shader->set_int("accumulation_texture", 0);

        wboit_fbo->bind_color_attachment(1); // revealage_texture -> unit 1
        composite_shader->set_int("revealage_texture", 1);

        Renderer::draw_quad();

        composite_shader->unuse();

        glDisable(GL_BLEND);

        // Unbind in the next pass(es)
    }
}

void RendererPasses::render_pass_post_processing()
{
    DebugScopeGroup scope("Post Processing Pass");

    if (GlobalSettings::get_global_setting_value<bool>(GlobalSettingOption::PostProcessing_Enabled))
    {
        PostProcessor::process_frame();
    }
}

void RendererPasses::render_pass_editor_overlays()
{
    DebugScopeGroup scope("Editor Overlays pass");

    // Editor-only visualization icon, drawn last so it's never covered by
    // transparency, and deliberately left with depth test disabled since
    // everything after this (post-processing) is screen-space only.
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (GlobalSettings::get_global_setting_value<bool>(GlobalSettingOption::Light_Enabled))
    {
        Scene::instance->get_active_light()->draw();
    }

    glDisable(GL_BLEND);
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
