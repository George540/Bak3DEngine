#include "command_buffer.h"

#include <vector>

#include "Core/logger.h"

using namespace std;

namespace
{
    GLsizei get_command_stride(const IndirectCommandType type)
    {
        switch (type)
        {
        case IndirectCommandType::DrawArraysIndirect:
        case IndirectCommandType::DrawMultiArraysIndirect:
            return DRAW_ARRAYS_INDIRECT_COMMAND_SIZE;

        case IndirectCommandType::DrawElementsIndirect:
        case IndirectCommandType::DrawMultiElementsIndirect:
            return DRAW_ELEMENTS_INDIRECT_COMMAND_SIZE;

        case IndirectCommandType::DispatchIndirect:
            return DISPATCH_INDIRECT_COMMAND_SIZE;
        }

        assert(false && "Invalid indirect command type");
        return 0;
    }
}

IndirectCommandBuffer::IndirectCommandBuffer(const GLsizei capacity, const IndirectCommandType type, const char* debug_name)
: Buffer(type == IndirectCommandType::DispatchIndirect ? GL_DISPATCH_INDIRECT_BUFFER : GL_DRAW_INDIRECT_BUFFER,
        static_cast<GLsizeiptr>(get_command_stride(type)) * capacity,
        nullptr,
        GL_DYNAMIC_DRAW),
        m_capacity(capacity),
        m_indirect_type(type)
{
    glObjectLabel(GL_BUFFER, m_ID, -1, debug_name);

    B3D_LOG_INFO("Indirect Command Buffer %s created %d commands.", debug_name, capacity);
}

void IndirectCommandBuffer::set_command(const GLsizei command_index, const DrawArraysIndirectCommand& command)
{
    assert(m_indirect_type == IndirectCommandType::DrawArraysIndirect || m_indirect_type == IndirectCommandType::DrawMultiArraysIndirect);
    assert(command_index >= 0);
    assert(command_index < m_capacity);

    bind();
    bind_buffer_sub_data(&command, DRAW_ARRAYS_INDIRECT_COMMAND_SIZE, static_cast<size_t>(command_index) * DRAW_ARRAYS_INDIRECT_COMMAND_SIZE);
}

void IndirectCommandBuffer::set_command(const GLsizei command_index, const DrawElementsIndirectCommand& command)
{
    assert(m_indirect_type == IndirectCommandType::DrawElementsIndirect || m_indirect_type == IndirectCommandType::DrawMultiElementsIndirect);
    assert(command_index >= 0);
    assert(command_index < m_capacity);

    bind();
    bind_buffer_sub_data(&command, DRAW_ELEMENTS_INDIRECT_COMMAND_SIZE, static_cast<size_t>(command_index) * DRAW_ELEMENTS_INDIRECT_COMMAND_SIZE);
}

void IndirectCommandBuffer::set_command(const GLsizei command_index, const DispatchIndirectCommand& command)
{
    assert(m_indirect_type == IndirectCommandType::DispatchIndirect);
    assert(command_index >= 0);
    assert(command_index < m_capacity);

    bind();
    bind_buffer_sub_data(&command, DISPATCH_INDIRECT_COMMAND_SIZE, static_cast<size_t>(command_index) * DISPATCH_INDIRECT_COMMAND_SIZE);
}

void IndirectCommandBuffer::reset_count(const GLsizei command_index)
{
    assert(m_indirect_type == IndirectCommandType::DrawArraysIndirect
        || m_indirect_type == IndirectCommandType::DrawElementsIndirect);
    assert(command_index >= 0);
    assert(command_index < m_capacity);
    
    static constexpr GLuint ZERO = 0;
    bind();
    bind_buffer_sub_data(&ZERO, UINT_SIZE, static_cast<size_t>(command_index) * get_command_stride(m_indirect_type));
}

void IndirectCommandBuffer::draw_command(const GLenum mode, const GLsizei command_index) const
{
    assert(m_indirect_type == IndirectCommandType::DrawArraysIndirect
        || m_indirect_type == IndirectCommandType::DrawElementsIndirect);
    assert(command_index >= 0);
    assert(command_index < m_capacity);

    bind();

    const void* offset = reinterpret_cast<const void*>(static_cast<intptr_t>(command_index) * get_command_stride(m_indirect_type));
    switch (m_indirect_type)
    {
        case IndirectCommandType::DrawArraysIndirect:
            glDrawArraysIndirect(mode, offset);
            break;
        case IndirectCommandType::DrawElementsIndirect:
            glDrawElementsIndirect(mode, GL_UNSIGNED_INT, offset);
            break;
        default:
            assert(false && "Invalid draw indirect command type");
            break;
    }
}

void IndirectCommandBuffer::draw_multi_command(const GLenum mode, const GLsizei draw_count, GLsizei stride) const
{
    assert(m_indirect_type == IndirectCommandType::DrawMultiArraysIndirect
        || m_indirect_type == IndirectCommandType::DrawMultiElementsIndirect);
    assert(draw_count >= 0);
    assert(draw_count <= m_capacity);

    bind();

    if (stride == 0)
    {
        stride = get_command_stride(m_indirect_type);
    }
    switch (m_indirect_type)
    {
        case IndirectCommandType::DrawMultiArraysIndirect:
            glMultiDrawArraysIndirect(mode, nullptr, draw_count, stride);
            break;
        case IndirectCommandType::DrawMultiElementsIndirect:
            glMultiDrawElementsIndirect(mode, GL_UNSIGNED_INT, nullptr, draw_count, stride);
            break;
        default:
            assert(false && "Invalid draw indirect command type");
            break;
    }
}

void IndirectCommandBuffer::dispatch_command(const GLsizei command_index) const
{
    assert(m_indirect_type == IndirectCommandType::DispatchIndirect);
    assert(command_index >= 0);
    assert(command_index < m_capacity);

    bind();

    const GLintptr offset = static_cast<GLintptr>(command_index) * get_command_stride(m_indirect_type);
    glDispatchComputeIndirect(offset);
}
