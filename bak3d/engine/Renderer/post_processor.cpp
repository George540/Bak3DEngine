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

#include "post_processor.h"

#include "post_process_pass.h"
#include "renderer.h"
#include "Buffers/vertex_array.h"
#include "Buffers/frame_buffer.h"
#include "Asset/resource_manager.h"
#include "Core/global_settings.h"
#include "Core/logger.h"
#include "Input/event_manager.h"

using namespace std;

namespace
{
    unique_ptr<FrameBuffer> m_fbo_a;
    unique_ptr<FrameBuffer> m_fbo_b;

    FrameBuffer* m_last_written_fbo;

    vector<unique_ptr<PostProcessPass>> m_passes;
}

void PostProcessor::initialize()
{
    const GLuint depth_texture = Renderer::get_main_frame_buffer()->get_depth_texture();
    m_fbo_a = make_unique<FrameBuffer>(EventManager::get_window_width(), EventManager::get_window_height(), depth_texture, "FrameBuffer_PP_A");
    m_fbo_b = make_unique<FrameBuffer>(EventManager::get_window_width(), EventManager::get_window_height(), depth_texture, "FrameBuffer_PP_B");

    // Register passes in execution order
    m_passes.push_back(make_unique<PostProcessPass_KernelEffect>(KernelEffectType::Sharpen));
    m_passes.push_back(make_unique<PostProcessPass_KernelEffect>(KernelEffectType::BoxBlur));
    m_passes.push_back(make_unique<PostProcessPass_KernelEffect>(KernelEffectType::Emboss));
    m_passes.push_back(make_unique<PostProcessPass_KernelEffect>(KernelEffectType::Sobel));
    m_passes.push_back(make_unique<PostProcessPass_KernelEffect>(KernelEffectType::Laplacian));
    m_passes.push_back(make_unique<PostProcessPass_ColorGrading>());

    B3D_LOG_INFO("Post Processing initialized...");
}

void PostProcessor::shutdown()
{
    m_passes.clear();

    m_fbo_a.reset();
    m_fbo_b.reset();
    m_last_written_fbo = nullptr;
}

void PostProcessor::process_frame()
{
    // Ping: Read from
    // Pong: Write to
    FrameBuffer* main_fbo = Renderer::get_main_frame_buffer();
    FrameBuffer* ping = main_fbo; // first read is the scene

    // Resolve main buffer to ping-pong buffers, so they start with the same input
    main_fbo->resolve_to(m_fbo_a.get());
    main_fbo->resolve_to(m_fbo_b.get());
    FrameBuffer* pong = m_fbo_a.get();

    bool first_write_to_a = true;

    for (const auto& pass : m_passes)
    {
        if (!pass->is_enabled())
        {
            continue;
        }

        const ShaderRef shader = pass->get_shader();

        pong->bind();
        shader->use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, static_cast<int>(ping->get_color_texture()));
        shader->set_int("screenTexture", 0);

        pass->process(); // let the pass upload its own uniforms

        Renderer::draw_quad();

        shader->unuse();

        // swap: last output becomes next input
        ping = pong;
        pong = first_write_to_a ? m_fbo_b.get() : m_fbo_a.get();
        first_write_to_a = !first_write_to_a;
    }

    m_last_written_fbo = ping; // the FBO holding the final result

    m_last_written_fbo->resolve_to(main_fbo);
    main_fbo->bind();
}

void PostProcessor::resize(const GLuint width, const GLuint height, const GLuint new_shared_depth_texture)
{
    m_fbo_a->resize(width, height, new_shared_depth_texture);
    m_fbo_b->resize(width, height, new_shared_depth_texture);
}
