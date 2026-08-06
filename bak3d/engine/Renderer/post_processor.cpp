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
#include "vertex_array.h"
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

    VertexArray* m_vao;
    VertexBuffer* m_vbo;
    ElementBuffer* m_ebo;
}

void PostProcessor::initialize()
{
    m_fbo_a = make_unique<FrameBuffer>(0, nullptr, EventManager::get_window_width(), EventManager::get_window_height());
    m_fbo_b = make_unique<FrameBuffer>(0, nullptr, EventManager::get_window_width(), EventManager::get_window_height());

    create_quad();

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

    destroy_quad();
}

void PostProcessor::process_frame(const FrameBuffer& resolved_fbo)
{
    // Ping: Read from
    // Pong: Write to
    FrameBuffer* ping = const_cast<FrameBuffer*>(&resolved_fbo); // first read is the scene
    FrameBuffer* pong = m_fbo_a.get();

    bool first_write_to_a = true;

    for (const auto& pass : m_passes)
    {
        if (!pass->is_enabled())
        {
            continue;
        }

        const ShaderRef shader = pass->get_shader();

        pong->bind_buffer();
        shader->use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, static_cast<int>(ping->get_render_buffer()));
        shader->set_int("screenTexture", 0);

        pass->process(); // let the pass upload its own uniforms

        draw_quad();

        shader->unuse();
        pong->unbind_buffer();

        // swap: last output becomes next input
        ping = pong;
        pong = first_write_to_a ? m_fbo_b.get() : m_fbo_a.get();
        first_write_to_a = !first_write_to_a;
    }

    m_last_written_fbo = ping; // the FBO holding the final result
}

FrameBuffer* PostProcessor::get_final_frame_buffer()
{
    return m_last_written_fbo;
}

void PostProcessor::resize(GLuint width, GLuint height)
{
    m_fbo_a->resize(width, height);
    m_fbo_b->resize(width, height);
}

void PostProcessor::create_quad()
{
    m_vao = new VertexArray();
    m_vao->bind_buffer();

    m_vbo = new VertexBuffer(static_cast<GLsizei>(QUAD_VERTICES.size()) * VEC4_SIZE, QUAD_VERTICES.data());
    m_ebo = new ElementBuffer(static_cast<GLsizei>(QUAD_INDICES.size()) * UINT_SIZE, QUAD_INDICES.data());

    m_vao->set_attrib_pointer(0, 4, GL_FLOAT, GL_FALSE, VEC4_SIZE, nullptr);
    m_vao->set_attrib_pointer(1, 2, GL_FLOAT, GL_FALSE, VEC4_SIZE, reinterpret_cast<void*>(2 * sizeof(float)));

    m_vao->unbind_buffer();
}

void PostProcessor::draw_quad()
{
    m_vao->bind_buffer();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(QUAD_INDICES.size()), GL_UNSIGNED_INT, nullptr);
    m_vao->unbind_buffer();
}

void PostProcessor::destroy_quad()
{
    delete m_ebo;
    delete m_vbo;
    delete m_vao;
}
