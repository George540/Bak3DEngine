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

#include "data_buffer.h"

#include "Core/logger.h"

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
