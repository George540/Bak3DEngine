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

#include <glad/glad.h>
#include <stdexcept>

#include "globject.h"

/*
 * Base class for OpenGL buffer OpenGL objects that contains data to be passed to the GPU.
 */
class Buffer: public GLObject
{
public:
    Buffer(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
    ~Buffer() override;

    void bind_object() const override;
    void unbind_object() const override;

    void set_buffer_data(const void* buffer_data, size_t buffer_data_size);
    void set_buffer_sub_data(const void* sub_data, size_t sub_data_size, size_t sub_data_offset);
protected:
    GLenum m_target;
    GLsizeiptr m_buffer_size;
    const void* m_data;
    GLenum m_usage;
};

/*
 * Buffer subclass for storing array buffer data
 */
class VertexBuffer : public Buffer
{
public:
    VertexBuffer(GLsizeiptr size, const void* data, GLenum usage = GL_STATIC_DRAW)
        : Buffer(GL_ARRAY_BUFFER, size, data, usage) {}
};

/*
 * Buffer subclass for storing element buffer data
 */
class ElementBuffer : public Buffer
{
public:
    ElementBuffer(GLsizeiptr size, const void* data, GLenum usage = GL_STATIC_DRAW)
        : Buffer(GL_ELEMENT_ARRAY_BUFFER, size, data, usage) {}
};

/*
 * Buffer subclass for storing instanced buffer data for instanced objects
 */
class InstanceBuffer : public Buffer
{
public:
    InstanceBuffer(GLsizeiptr size, const void* data, GLenum usage = GL_STATIC_DRAW)
        : Buffer(GL_ARRAY_BUFFER, size, data, usage) {}

    void set_size(int size) { m_buffer_size = size; }
};

/*
 * Framebuffer class that stores generated output textures with rendering buffer objects.
 * Default framebuffer for OpenGL has ID 0, which is rendering a texture in the full viewport window.
 */
class FrameBuffer : public Buffer
{
public:
    FrameBuffer(GLsizeiptr size, const void* data, const GLuint width, const GLuint height, GLenum usage = GL_NONE);
    ~FrameBuffer() override;
    void bind_object() const override;
    void unbind_object() const override;
    void resize(GLuint new_width, GLuint new_height);

    GLuint get_width() const { return m_width; }
    GLuint get_height() const { return m_height; }
    float get_aspect_ratio() const;
    GLuint get_render_buffer() const { return m_texture_buffer; }
    GLuint get_depth_buffer() const { return m_rbo; }

protected:
    virtual void create_attachments();
    void create_framebuffer();
    void destroy_framebuffer();

    GLuint m_width;
    GLuint m_height;
    GLuint m_texture_buffer;
    GLuint m_rbo;
};

/*
 * A FrameBuffer variant that stores sample sizes for Multisampled Anti-Aliasing
 */
class MultisampleFrameBuffer : public FrameBuffer
{
public:
    MultisampleFrameBuffer(GLsizeiptr size, const void* data, GLuint width, GLuint height, GLsizei samples = 4, GLenum usage = GL_NONE);

    void set_samples(const GLsizei new_samples);
    GLsizei get_samples() const { return m_samples; }
    GLsizei get_max_samples () const { return m_max_samples; }

    void resolve_to(const FrameBuffer& fbo_target) const;
protected:
    void create_attachments() override;

private:
    GLsizei m_samples;
    GLsizei m_max_samples;
};

/*
 * Abstract buffer class for data-oriented buffer objects, that use binding points.
 * Accessed via arbitrary user-defined layouts (std140/std430).
 */
class DataBuffer : public Buffer
{
public:
    DataBuffer(GLenum target, GLsizeiptr size, const void* data, const GLuint index, GLenum usage = GL_STATIC_DRAW) : Buffer(target, size, data, usage) {}
    virtual void bind_to_binding_point(GLuint index) const = 0;
};

/*
 * An extremely fast and lightweight read-only GPU memory buffer for storing constant variables shared across multiple shaders.
 * Its capacity is small (16-64KB) and data are strictly structured using std140, typically in vec4 format.
 */
class UniformBuffer : public DataBuffer
{
public:
    UniformBuffer(GLsizeiptr size, const void* data, GLuint index, GLenum usage = GL_STATIC_DRAW);
    void bind_to_binding_point(GLuint index) const override;
};

/*
 * Shader Storage Buffer Object. No 64KB cap like UBOs and have both read and write access.
 * Supports arbitrarily large buffers and variable-length arrays (unlike constant vec4 format).
 * Used for large draw data, such as particles, materials and other.
 */
class ShaderStorageBuffer : public DataBuffer
{
public:
    ShaderStorageBuffer(GLsizeiptr size, const void* data, GLuint index, GLenum usage = GL_DYNAMIC_DRAW);
    void bind_to_binding_point(GLuint index) const override;
};