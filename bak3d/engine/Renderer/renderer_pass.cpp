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
#include "light_renderer.h"
#include "post_processor.h"
#include "renderer.h"
#include "shadow_renderer.h"
#include "Asset/resource_manager.h"
#include "Core/global_settings.h"
#include "Scene/scene_manager.h"

namespace
{
    bool is_deferred_tagged(const Mesh* mesh)
    {
        const MaterialRef& material = mesh->get_material();
        if (!material)
        {
            return false;
        }
        const ShaderRef& shader = material->get_shader();
        return shader && shader->get_object_name() == "gbuffer";
    }

    bool is_editor_view()
    {
        const Camera* camera = SceneManager::get_current_scene()->get_current_camera();
        return camera && camera->m_is_editor_only;
    }
}

void RendererPasses::render_pass_shadow_map()
{
    DebugScopeGroup scope("Shadow Map Pass");

    ShadowRenderer::render();
}

void RendererPasses::render_pass_gbuffer()
{
    DebugScopeGroup scope("GBuffer Pass (Deferred Opaque)");

    for (const Mesh* mesh : SceneManager::get_current_scene()->get_all_meshes())
    {
        if (is_deferred_tagged(mesh))
        {
            mesh->draw();
        }
    }
}

void RendererPasses::render_pass_light_culling()
{
    DebugScopeGroup scope("Light Culling");

    if (const Camera* camera = SceneManager::get_current_scene()->get_current_camera())
    {
        LightRenderer::update_and_upload_data(SceneManager::get_current_scene()->get_all_lights(), camera->get_frustum(), camera->get_camera_position());
    }
}

void RendererPasses::render_pass_deferred_lighting()
{
    DebugScopeGroup scope("Deferred Lighting Pass");

    const ShaderRef shader = ResourceManager::get_shader("deferred_lit");
    if (!shader || !shader->is_shader_compiled())
    {
        return;
    }

    const GBufferFrameBuffer* gbuffer = Renderer::get_gbuffer();

    shader->use();

    //gbuffer->bind_color_attachment(0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gbuffer->get_position_texture());
    shader->set_int("g_position", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gbuffer->get_normals_texture());
    shader->set_int("g_normal", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gbuffer->get_albedo_spec_texture());
    shader->set_int("g_albedo_specular", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gbuffer->get_material_texture());
    shader->set_int("g_material", 3);

    shader->set_int("light_count", LightRenderer::get_visible_light_count());

    Renderer::draw_quad();

    shader->unuse();
}

void RendererPasses::render_pass_forward_opaque()
{
    DebugScopeGroup scope("Forward Opaque Pass");

    for (const Mesh* mesh : SceneManager::get_current_scene()->get_all_meshes())
    {
        if (!is_deferred_tagged(mesh))
        {
            mesh->draw();
        }
    }
}

void RendererPasses::render_pass_debug_geometry()
{
    if (!is_editor_view())
    {
        return;
    }
    
    DebugScopeGroup scope("Debug Geometry Pass");

    glDepthFunc(GL_LEQUAL);

    if (GlobalSettings::get_global_setting_value<uint32_t>(GlobalSettingOption::EditorOverlayFlags) & static_cast<uint32_t>(OverlaysFlags::WorldGrid))
    {
        for (const auto& debug_geometry : SceneManager::get_current_scene()->get_all_debug_geometry())
        {
            debug_geometry->draw();
        }
    }

    glDepthFunc(GL_LESS);
}

void RendererPasses::render_pass_opaque_geometry()
{
    DebugScopeGroup scope("Opaque Geometry Pass");

    glDepthMask(GL_TRUE);

    for (const RenderableObject* obj : SceneManager::get_current_scene()->get_all_meshes())
    {
        obj->draw();
    }
}

void RendererPasses::render_pass_sprites()
{
    DebugScopeGroup scope("Sprites Pass");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    for (const ParticleSystem* particle_system : SceneManager::get_current_scene()->get_all_particle_systems())
    {
        particle_system->draw();
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
}

void RendererPasses::render_pass_transparency()
{
    DebugScopeGroup scope("Transparency Pass (WBOIT)");

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

        glBlendFunci(0, GL_ONE, GL_ONE); // accumulation target: additive
        glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR); // revealage target: product of (1 - alpha)
        glBlendEquation(GL_FUNC_ADD);

        for (const AdvancedParticleSystem* advanced_particle_system : SceneManager::get_current_scene()->get_all_advanced_particle_systems())
        {
            advanced_particle_system->draw();
        }

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

void RendererPasses::render_pass_skybox()
{
    DebugScopeGroup scope("Skybox Pass");

    if (!GlobalSettings::get_global_setting_value<bool>(GlobalSettingOption::Skybox_Enabled))
    {
        return;
    }

    const int mode_index = GlobalSettings::get_global_setting_value<int>(GlobalSettingOption::Skybox_Mode);
    if (mode_index < 0 || mode_index >= static_cast<int>(SkyboxMode::Count))
    {
        return;
    }

    const SkyboxModeInfo& info = SkyboxModeInfos[mode_index];
    if (!info.implemented || !ResourceManager::Materials.contains(info.material_name))
    {
        return;
    }

    const MaterialRef material = ResourceManager::get_material(info.material_name);
    const MeshRef cube = ResourceManager::get_mesh("Cube");
    if (!material || !cube || !material->get_shader() || !material->get_shader()->is_shader_compiled())
    {
        return;
    }

    TextureRef cubemap;
    switch (static_cast<SkyboxMode>(mode_index))
    {
    case SkyboxMode::Cubemap:
    {
        const string cubemap_file_name = GlobalSettings::get_global_setting_value<string>(GlobalSettingOption::Skybox_Cubemap);
        string cubemap_name = FileLoader::get_name_from_filename(cubemap_file_name);
        if (!ResourceManager::Cubemaps.contains(cubemap_name))
        {
            if (cubemap_name.empty())
            {
                cubemap_name = material->get_texture_by_type(aiTextureType_DIFFUSE);
            }
            if (ResourceManager::Cubemaps.empty())
            {
                return;
            }
            cubemap_name = ResourceManager::Cubemaps.all().begin()->first;
            GlobalSettings::set_global_setting<string>(GlobalSettingOption::Skybox_Cubemap, cubemap_name);
        }
        cubemap = ResourceManager::get_cubemap(cubemap_name);
        break;
    }
    default:
        return;
    }

    // Common parameters (same Y * X * Z order as Transform; transpose = inverse for rotating the sample direction)
    const glm::vec3 euler = GlobalSettings::get_global_setting_value<glm::vec3>(GlobalSettingOption::Skybox_Rotation);
    const glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(euler.y), glm::vec3(0.0f, 1.0f, 0.0f))
                             * glm::rotate(glm::mat4(1.0f), glm::radians(euler.x), glm::vec3(1.0f, 0.0f, 0.0f))
                             * glm::rotate(glm::mat4(1.0f), glm::radians(euler.z), glm::vec3(0.0f, 0.0f, 1.0f));
    material->set_int("cubemap", 0);
    material->set_vec3("color", GlobalSettings::get_global_setting_value<glm::vec3>(GlobalSettingOption::Skybox_Color));
    material->set_float("intensity", GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::Skybox_Intensity));
    material->set_mat3("sky_rotation", glm::transpose(glm::mat3(rotation)));

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL); // sky depth == 1.0 
    glDepthMask(GL_FALSE); // never write depth
    glDisable(GL_CULL_FACE); // inside view of cube

    material->apply();
    cubemap->bind(0);
    cube->draw();

    Texture2D::unbind(0, cubemap->get_target());
    material->get_shader()->unuse();

    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
}

void RendererPasses::render_pass_post_processing()
{
    DebugScopeGroup scope("Post Processing Pass");

    PostProcessor::process_frame();
}

void RendererPasses::render_pass_editor_overlays()
{
    DebugScopeGroup scope("Editor Overlays pass");

    if (!is_editor_view())
    {
        return;
    }

    const uint32_t flags = GlobalSettings::get_global_setting_value<uint32_t>(GlobalSettingOption::EditorOverlayFlags);
    const SceneObject* selected = SceneManager::get_current_scene()->get_selected_scene_object();

    const auto* selected_light = dynamic_cast<const Light*>(selected);
    const auto* selected_camera = dynamic_cast<const Camera*>(selected);
    const bool draw_light = (flags & static_cast<uint32_t>(OverlaysFlags::LightGeometry)) && selected_light && selected_light->is_active;
    const bool draw_camera = (flags & static_cast<uint32_t>(OverlaysFlags::CameraGeometry)) && selected_camera && !selected_camera->m_is_editor_only && selected_camera->is_active;

    if (draw_light || draw_camera)
    {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_FALSE);
        if (draw_light) selected_light->draw_debug_geometry();
        if (draw_camera) selected_camera->draw_debug_geometry();
        glDepthMask(GL_TRUE);
    }

    // Depth-Tested Light Gizmos
    if (GlobalSettings::get_global_setting_value<uint32_t>(GlobalSettingOption::EditorOverlayFlags) & static_cast<uint32_t>(OverlaysFlags::LightGeometry))
    {
        const SceneObject* selected = SceneManager::get_current_scene()->get_selected_scene_object();
        if (const auto* selected_light = dynamic_cast<const Light*>(selected); selected_light && selected_light->is_active)
        {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            glDepthMask(GL_FALSE);

            selected_light->draw_debug_geometry();

            glDepthMask(GL_TRUE);
        }
    }

    // Editor-only world positioned visualization icon, with depth test disabled.
    // Not affected by any other pass, except UI.
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (GlobalSettings::get_global_setting_value<uint32_t>(GlobalSettingOption::EditorOverlayFlags) & static_cast<uint32_t>(OverlaysFlags::LightIcons))
    {
        for (const Light* light : SceneManager::get_current_scene()->get_all_lights())
        {
            light->draw();
        }
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

    const auto view_mode = static_cast<DebugViewMode>(GlobalSettings::get_global_setting_value<int>(GlobalSettingOption::ViewMode));

    Renderer::get_debug_view_buffer()->bind();

    debug_view_shader->use();

    GLuint active_view_texture = 0;
    switch (view_mode)
    {
    case DebugViewMode::GBuffer_Position:
        active_view_texture = Renderer::get_gbuffer()->get_position_texture();
        break;
    case DebugViewMode::GBuffer_Albedo:
    case DebugViewMode::GBuffer_Specular:
        active_view_texture = Renderer::get_gbuffer()->get_albedo_spec_texture();
        break;
    case DebugViewMode::GBuffer_Normals:
        active_view_texture = Renderer::get_gbuffer()->get_normals_texture();
        break;
    case DebugViewMode::Depth:
        active_view_texture = Renderer::get_main_frame_buffer()->get_depth_texture();
        break;
    case DebugViewMode::ShadowMask:
        active_view_texture = Renderer::get_gbuffer()->get_position_texture();
        break;
    default:
        break;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, active_view_texture);
    debug_view_shader->set_int("debug_view_texture", 0);

    if (view_mode == DebugViewMode::ShadowMask)
    {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, Renderer::get_gbuffer()->get_normals_texture());
        debug_view_shader->set_int("debug_view_normal_texture", 1);
        debug_view_shader->set_int("active_light_count", LightRenderer::get_visible_light_count());
        glActiveTexture(GL_TEXTURE0);
    }

    Renderer::draw_quad();

    debug_view_shader->unuse();

    Renderer::get_debug_view_buffer()->unbind();
}
