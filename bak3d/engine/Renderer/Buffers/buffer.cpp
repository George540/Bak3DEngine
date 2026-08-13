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
