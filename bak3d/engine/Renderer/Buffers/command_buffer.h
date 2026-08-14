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

enum class IndirectCommandType : uint8_t
{
    DrawArraysIndirect = 0,
    DrawElementsIndirect,
    DrawMultiArraysIndirect,
    DrawMultiElementsIndirect,
    DispatchIndirect,
};

/*
 * Struct of Indirect Draw and Dispatch commands info to be queued and collected by the GPU.
 */
struct DrawArraysIndirectCommand
{
    GLuint count = 0; // Number of vertices
    GLuint instance_count = 1; // How many instances
    GLuint first = 0; // First in the index array
    GLuint base_instance = 0; // 
};
static constexpr GLsizei DRAW_ARRAYS_INDIRECT_COMMAND_SIZE = sizeof(DrawArraysIndirectCommand);
static_assert(DRAW_ARRAYS_INDIRECT_COMMAND_SIZE == 16, "Must match 4xGLuint DrawArraysIndirectCommand layout");


struct DrawElementsIndirectCommand
{
    GLuint count = 0;
    GLuint instance_count = 1;
    GLuint first_index = 0;
    GLuint base_vertex = 0; // First in the vertex array
    GLuint base_instance = 0; // First in the instance array
};
static constexpr GLsizei DRAW_ELEMENTS_INDIRECT_COMMAND_SIZE = sizeof(DrawElementsIndirectCommand);
static_assert(DRAW_ELEMENTS_INDIRECT_COMMAND_SIZE == 20, "Must match 5xGLuint DrawElementsIndirectCommand layout");

struct DispatchIndirectCommand
{
    GLuint work_group_size_x = 1;
    GLuint work_group_size_y = 1;
    GLuint work_group_size_z = 1;
};
static constexpr GLsizei DISPATCH_INDIRECT_COMMAND_SIZE = sizeof(DispatchIndirectCommand);
static_assert(DISPATCH_INDIRECT_COMMAND_SIZE == 12, "Must match 3xGLuint DispatchIndirectCommand layout");

/*
 * Buffer holding one or more indirect draw commands to be sent to the GPU
 */
class IndirectCommandBuffer : public Buffer
{
public:
    explicit IndirectCommandBuffer(GLsizei capacity, IndirectCommandType type, const char* debug_name = nullptr);
    
    void set_command(GLsizei command_index, const DrawArraysIndirectCommand& command);
    void set_command(GLsizei command_index, const DrawElementsIndirectCommand& command);
    void set_command(GLsizei command_index, const DispatchIndirectCommand& command);

    void reset_count(GLsizei command_index);

    // Dual-purpose binding. SSBO format can be used to inject into a specific command's data, such as the capacity.
    void bind_as_ssbo(const GLuint binding_index) const { glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_index, m_ID); }
    void draw_command(GLenum mode, GLsizei command_index = 0) const;
    void draw_multi_command(GLenum mode, GLsizei draw_count, GLsizei stride = 0) const;
    void dispatch_command(GLsizei command_index) const;

    GLsizei get_capacity() const { return m_capacity; }
    IndirectCommandType get_command_type() const { return m_indirect_type; }
private:
    GLsizei m_capacity; // How many entities can share the same buffer data (multi-draw)
    IndirectCommandType m_indirect_type;
};
