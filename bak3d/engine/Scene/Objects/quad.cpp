#include "quad.h"

Quad::Quad() :RenderableObject(nullptr, glm::vec3(0.0f, 0.0f, 0.0f), "Quad")
{
    m_vao = new VertexArray();
    m_vao->bind();

    m_vbo = new VertexBuffer(static_cast<GLsizei>(QUAD_VERTICES.size()) * VEC4_SIZE, QUAD_VERTICES.data());
    m_ebo = new ElementBuffer(static_cast<GLsizei>(QUAD_INDICES.size()) * UINT_SIZE, QUAD_INDICES.data());

    m_vao->set_attrib_pointer(0, 4, GL_FLOAT, GL_FALSE, VEC4_SIZE, nullptr);
    m_vao->set_attrib_pointer(1, 2, GL_FLOAT, GL_FALSE, VEC4_SIZE, reinterpret_cast<void*>(2 * sizeof(float)));

    m_vao->unbind();
}

void Quad::draw() const
{
    m_vao->bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(QUAD_INDICES.size()), GL_UNSIGNED_INT, nullptr);
    m_vao->unbind();
}
