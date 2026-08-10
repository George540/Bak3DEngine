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

#include "buffer.h"

#include <iostream>
#include <utility>

#include "Core/global_definitions.h"
#include "Core/logger.h"

using namespace std;

Buffer::Buffer(GLenum target, GLsizeiptr size, const void* data, GLenum usage) :
    m_target(target),
    m_buffer_size(size),
    m_data(data),
    m_usage(usage)
{
    // TODO: make constructor arguments more defined
    if (m_usage != GL_NONE)
    {
        glGenBuffers(1, &m_ID);
        glBindBuffer(m_target, m_ID);
        bind_buffer_data(m_data, m_buffer_size);
    }
}

Buffer::~Buffer()
{
    glDeleteBuffers(1, &m_ID);
}

void Buffer::bind() const
{
    glBindBuffer(m_target, m_ID);
}

void Buffer::unbind() const
{
    glBindBuffer(m_target, 0);
}

void Buffer::bind_buffer_data(const void* buffer_data, const size_t buffer_data_size)
{
    if (m_data != buffer_data || cmp_not_equal(m_buffer_size, buffer_data_size))
    {
        m_data = buffer_data;
        m_buffer_size = buffer_data_size;
    }
    glBufferData(m_target, m_buffer_size, m_data, m_usage);
}

void Buffer::bind_buffer_sub_data(const void* sub_data, const size_t sub_data_size, const size_t sub_data_offset)
{
    if (sub_data_offset + sub_data_size > m_buffer_size)
    {
        // Prevent out-of-bounds GPU memory write
        throw runtime_error("Buffer overflow: bind_buffer_sub_data(...) exceeds allocated size");
    }
    glBufferSubData(m_target, sub_data_offset, sub_data_size, sub_data);
}

void Buffer::insert_memory_barrier(GLbitfield bit_fields)
{
    if (bit_fields != 0)
    {
        glMemoryBarrier(bit_fields);
    }
}

FrameBuffer::FrameBuffer(
    GLsizeiptr size,
    const void* data,
    const GLuint width,
    const GLuint height,
    const GLenum attachment_type,
    const GLenum usage,
    const bool use_depth_texture,
    const char* debug_name)
    : Buffer(GL_FRAMEBUFFER, size, data, usage),
    m_width(width),
    m_height(height),
    m_attachment_type(attachment_type),
    m_use_depth_texture(use_depth_texture),
    m_debug_name(debug_name ? debug_name : "FrameBuffer")
{
    create_framebuffer();
    
    B3D_LOG_INFO("Frame Buffer Object '%s' with ID %d enabled (%s depth)...", m_debug_name.c_str(), m_ID, m_use_depth_texture ? "sampled" : "renderbuffer");
}

FrameBuffer::~FrameBuffer()
{
    destroy_framebuffer();
}

void FrameBuffer::bind() const
{
    glBindFramebuffer(m_target, m_ID);
    glViewport(0, 0, m_width, m_height);
}

void FrameBuffer::unbind() const // when unbinding a frame buffer, it binds back to default
{
    glBindFramebuffer(m_target, 0);
    glViewport(0, 0, m_width, m_height);
}

void FrameBuffer::resize(GLuint new_width, GLuint new_height)
{
    if (m_width != new_width || m_height != new_height)
    {
        m_width = new_width;
        m_height = new_height;

        destroy_framebuffer();
        create_framebuffer();
    }
}

float FrameBuffer::get_aspect_ratio() const
{
    if (m_height == 0)
    {
        return 1.0f;
    }
    return static_cast<float>(m_width) / static_cast<float>(m_height);
}

void FrameBuffer::create_attachments()
{
    glGenTextures(1, &m_color_texture);
    glBindTexture(GL_TEXTURE_2D, m_color_texture);

    if (m_attachment_type == GL_COLOR_ATTACHMENT0)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(m_target, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_texture, 0);

        if (m_use_depth_texture)
        {
            glGenTextures(1, &m_depth_texture);
            glBindTexture(GL_TEXTURE_2D, m_depth_texture);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_width, m_height, 0,
                         GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
            
            glFramebufferTexture2D(m_target, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depth_texture, 0);
        }
        else
        {
            glGenRenderbuffers(1, &m_rbo);
            glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
            glFramebufferRenderbuffer(m_target, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);
        }
    }
    else if (m_attachment_type == GL_DEPTH_ATTACHMENT)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, SWIZZLE_MASK);

        glFramebufferTexture2D(m_target, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_color_texture, 0);

        // Explicitly disable color writes for depth buffer
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    label_resources();
}

void FrameBuffer::label_resources() const
{
    glObjectLabel(GL_FRAMEBUFFER, m_ID, -1, m_debug_name.c_str());

    const string color_label = m_debug_name + "_Color";
    glObjectLabel(GL_TEXTURE, m_color_texture, -1, color_label.c_str());

    if (m_use_depth_texture)
    {
        const string depth_label = m_debug_name + "_Depth";
        glObjectLabel(GL_TEXTURE, m_depth_texture, -1, depth_label.c_str());
    }
    else if (m_rbo != 0)
    {
        const string depth_label = m_debug_name + "_DepthStencilRBO";
        glObjectLabel(GL_RENDERBUFFER, m_rbo, -1, depth_label.c_str());
    }
}

void FrameBuffer::create_framebuffer()
{
    if (m_target != GL_FRAMEBUFFER)
    {
        B3D_LOG_ERROR("Framebuffer target is not correct. Must be GL_FRAMEBUFFER.");
    }

    glGenFramebuffers(1, &m_ID);
    glBindFramebuffer(m_target, m_ID);

    create_attachments();

    // Verify framebuffer is complete
    if (GLenum status = glCheckFramebufferStatus(m_target); status != GL_FRAMEBUFFER_COMPLETE)
    {
        B3D_LOG_ERROR("Framebuffer is not complete! Status: 0x%x", status);
    }

    glBindFramebuffer(m_target, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void FrameBuffer::destroy_framebuffer()
{
    if (m_ID != 0)
    {
        glDeleteFramebuffers(1, &m_ID);
        m_ID = 0;
    }
    if (m_color_texture != 0)
    {
        glDeleteTextures(1, &m_color_texture);
        m_color_texture = 0;
    }
    if (m_rbo != 0)
    {
        glDeleteRenderbuffers(1, &m_rbo);
        m_rbo = 0;
    }
    if (m_depth_texture != 0)
    {
        glDeleteTextures(1, &m_depth_texture);
        m_depth_texture = 0;
    }
}

MultisampleFrameBuffer::MultisampleFrameBuffer(
    GLsizeiptr size,
    const void* data,
    GLuint width,
    GLuint height,
    GLsizei samples,
    GLenum usage,
    const char* debug_name)
    : FrameBuffer(size, data, width, height, GL_COLOR_ATTACHMENT0, usage, false, debug_name),
    m_samples(samples),
    m_max_samples(0)
{
    glGetIntegerv(GL_MAX_SAMPLES, &m_max_samples);

    // Validate the requested samples against the hardware limit.
    // Fallback to a safe default if invalid.
    if (samples < 1 || samples > m_max_samples || (samples & (samples - 1)) != 0)
    {
        B3D_LOG_ERROR("Invalid MSAA sample count: %d. Must be power of two or greater than the hardware limit: %d.", samples, m_max_samples);
        m_samples = 4;
    }
    // FrameBuffer constructor already called create_framebuffer(). Redo it for this one.
    destroy_framebuffer();
    create_framebuffer();

    B3D_LOG_INFO("MSAA Frame Buffer Object enabled...");
}

void MultisampleFrameBuffer::set_samples(const GLsizei new_samples)
{
    if (new_samples < 1 || new_samples > m_max_samples || (new_samples & (new_samples - 1)) != 0)
    {
        B3D_LOG_ERROR("Invalid sample count: %d. Must be a power of two between 1 and 8.", new_samples);
        return;
    }
    if (m_samples != new_samples)
    {
        m_samples = new_samples;
        destroy_framebuffer();
        create_framebuffer();
        
        B3D_LOG_INFO("Resampling MSAA Frame Buffer Object to %dx%d samples...", m_samples, m_samples);
    }
}

void MultisampleFrameBuffer::resolve_to(const FrameBuffer& fbo_target) const
{
    // Ensure both intermediate FBO and MSAA FBO are the same size.
    assert(m_width == fbo_target.get_width() && m_height == fbo_target.get_height());

    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_ID);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_target.get_id());
    glBlitFramebuffer(
        0, 0, m_width, m_height,
        0, 0, fbo_target.get_width(), fbo_target.get_height(),
        GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
        GL_NEAREST
    );

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void MultisampleFrameBuffer::create_attachments()
{
    glGenTextures(1, &m_color_texture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_color_texture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_samples, GL_RGB, m_width, m_height, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glFramebufferTexture2D(m_target, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_color_texture, 0);

    glGenRenderbuffers(1, &m_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_samples, GL_DEPTH24_STENCIL8, m_width, m_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(m_target, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);

    label_resources();
}

UniformBuffer::UniformBuffer(GLsizeiptr size, const void* data, const GLuint index, GLenum usage)
    : DataBuffer(GL_UNIFORM_BUFFER, size, data, usage)
{
    Buffer::unbind();
    glBindBufferRange(GL_UNIFORM_BUFFER, index, m_ID, 0, m_buffer_size);
    B3D_LOG_INFO("Uniform Buffer Object enabled...");
}

void UniformBuffer::bind_to_binding_point(const GLuint index) const
{
    glBindBufferRange(GL_UNIFORM_BUFFER, index, m_ID, 0, m_buffer_size);
}

ShaderStorageBuffer::ShaderStorageBuffer(GLsizeiptr size, const void* data, const GLuint index, GLenum usage)
    : DataBuffer(GL_SHADER_STORAGE_BUFFER, size, data, index, usage)
{
    Buffer::unbind();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, m_ID);
    B3D_LOG_INFO("Shader Storage Buffer Object enabled: %lld bytes at binding %u", size, index);
}

void ShaderStorageBuffer::bind_to_binding_point(const GLuint index) const
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, m_ID);
}

void ShaderStorageBuffer::reset(const GLenum internal_fmt, const GLenum base_fmt, const GLenum type) const
{
    bind();

    constexpr GLuint value = 0;
    glClearBufferData(GL_SHADER_STORAGE_BUFFER, internal_fmt, base_fmt, type, &value);

    unbind();
}

AtomicCounterBuffer::AtomicCounterBuffer(GLuint binding_index, GLenum usage)
    : DataBuffer(GL_ATOMIC_COUNTER_BUFFER, UINT_SIZE, nullptr, usage)
{
    // Initialise counter to 0
    constexpr GLuint zero = 0;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_ID);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, UINT_SIZE, &zero, usage);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, binding_index, m_ID);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
    B3D_LOG_INFO("Atomic Counter Buffer created at binding %u", binding_index);
}

void AtomicCounterBuffer::bind_to_binding_point(GLuint binding_index) const
{
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, binding_index, m_ID);
}

void AtomicCounterBuffer::set_counter(const GLuint value) const
{
    bind();

    // glMapBufferRange is preferred over glBufferSubData for atomics, because it avoids implicit synchronisation on some drivers
    if (GLuint* ptr = static_cast<GLuint*>(glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, UINT_SIZE, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT)))
    {
        *ptr = value;
        glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    }

    unbind();
}

GLuint AtomicCounterBuffer::read_counter() const
{
    bind();

    GLuint value = 0;
    if (const GLuint* ptr = static_cast<GLuint*>(glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, UINT_SIZE, GL_MAP_READ_BIT)))
    {
        value = *ptr;
        glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    }

    unbind();

    return value;
}
