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

#include "buffer.h"

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
    void reset(GLenum internal_fmt, GLenum base_fmt, GLenum type) const;
};

/*
 * A thread-safe, specialized buffer that is race-free across all GPU cores.
 * Stores atomic 32-bit integers (uint) that is hardware optimized.
 * Faster operations than a standard Shader Storage Buffer.
 */
class AtomicCounterBuffer : public DataBuffer
{
public:
    AtomicCounterBuffer(GLuint binding_index, GLenum usage = GL_DYNAMIC_DRAW);
    void bind_to_binding_point(GLuint binding_index) const override;

    void set_counter(GLuint value = 0) const;
    GLuint read_counter() const;
};
