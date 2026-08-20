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

#include "mesh_data.h"

using namespace std;

PrimitiveData::PrimitiveData(const string& name)
    : Asset("", "") // No file data, since it is a transient resource (might add original model data)
{
    m_object_name = name;

    m_vao = new VertexArray();
    m_vao->bind();
}

MeshData::MeshData(vector<Vertex> vertices, vector<GLuint> indices, const string& name)
    : PrimitiveData(name), m_vertices(move(vertices)), m_indices(move(indices))
{
    m_vbo = new VertexBuffer(VERTEX_SIZE * m_vertices.size(), m_vertices.data());
    m_ebo = new ElementBuffer(UINT_SIZE * m_indices.size(), m_indices.data());

    m_vao->set_attrib_pointer(0, 3, GL_FLOAT, GL_FALSE, VERTEX_SIZE, nullptr);
    m_vao->set_attrib_pointer(1, 3, GL_FLOAT, GL_FALSE, VERTEX_SIZE, reinterpret_cast<void*>(offsetof(Vertex, normal)));
    m_vao->set_attrib_pointer(2, 2, GL_FLOAT, GL_FALSE, VERTEX_SIZE, reinterpret_cast<void*>(offsetof(Vertex, tex_coords)));
    m_vao->set_attrib_pointer(3, 3, GL_FLOAT, GL_FALSE, VERTEX_SIZE, reinterpret_cast<void*>(offsetof(Vertex, tangent)));
    m_vao->set_attrib_pointer(4, 3, GL_FLOAT, GL_FALSE, VERTEX_SIZE, reinterpret_cast<void*>(offsetof(Vertex, bitangent)));
    m_vao->set_attrib_pointer(5, 4, GL_INT, GL_FALSE, VERTEX_SIZE, reinterpret_cast<void*>(offsetof(Vertex, m_BoneIDs)));
    m_vao->set_attrib_pointer(6, 4, GL_FLOAT, GL_FALSE, VERTEX_SIZE, reinterpret_cast<void*>(offsetof(Vertex, m_Weights)));

    m_vao->unbind();

    m_index_count = static_cast<GLsizei>(m_indices.size());
}

QuadData::QuadData() : PrimitiveData("Quad")
{
    m_vbo = new VertexBuffer(static_cast<GLsizei>(QUAD_VERTICES.size()) * VEC4_SIZE, QUAD_VERTICES.data());
    m_ebo = new ElementBuffer(static_cast<GLsizei>(QUAD_INDICES.size()) * UINT_SIZE, QUAD_INDICES.data());

    // <vec2: vertex position , vec2: texture coordinates>
    m_vao->set_attrib_pointer(0, 4, GL_FLOAT, GL_FALSE, VEC4_SIZE, nullptr);
    m_vao->unbind();

    m_index_count = static_cast<GLsizei>(QUAD_INDICES.size());
}

GridData::GridData() : GridData(40, 20.0f) {}

GridData::GridData(const int num_of_elements, const float grid_size) : PrimitiveData("Grid"), m_number_of_slices(num_of_elements), m_grid_size(grid_size)
{
    // GRID LINE SETUP
    // Grid setup used indices and EBOs for proper identification of each line
    // Part of the solution is referenced from thewoz's answer on stackoverflow.com
    // Link to question and answer: https://stackoverflow.com/questions/58494179/how-to-create-a-grid-in-opengl-and-drawing-it-with-lines#:~:text=this%20the%20code%20for%20render,opengl
    vector<glm::vec3> vertices;
    vector<glm::uvec4> indices;

    // Set up vertex positions
    for (int j = 0; j <= m_number_of_slices; ++j) {
        for (int i = 0; i <= m_number_of_slices; ++i) {
            // Calculate XZ plane coordinates
            const float x = static_cast<float>(i) / static_cast<float>(m_number_of_slices);
            const float z = static_cast<float>(j) / static_cast<float>(m_number_of_slices);
            vertices.emplace_back(x * m_grid_size - m_grid_size / 2.0f, 0, z * m_grid_size - m_grid_size / 2.0f);
        }
    }

    // Set up indices for Index Buffer Object for element buffer
    for (int j = 0; j < m_number_of_slices; ++j) {
        for (int i = 0; i < m_number_of_slices; ++i) {
            const int row1 = j * (m_number_of_slices + 1);
            const int row2 = (j + 1) * (m_number_of_slices + 1);

            indices.emplace_back(row1 + i, row1 + i + 1, row1 + i + 1, row2 + i + 1);
            indices.emplace_back(row2 + i + 1, row2 + i, row2 + i, row1 + i);
        }
    }

    m_vbo = new VertexBuffer(VEC3_SIZE * vertices.size(), vertices.data());
    m_ebo = new ElementBuffer(UVEC4_SIZE * indices.size(), indices.data());

    m_vao->set_attrib_pointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    m_vao->set_attrib_pointer(1, 3, GL_FLOAT, GL_FALSE, 2 * VEC3_SIZE, reinterpret_cast<void*>(VEC3_SIZE));
    m_vao->unbind();

    m_index_count = indices.size() * 4;
    m_draw_mode = GL_LINES;
}

BoundingBoxData::BoundingBoxData() : PrimitiveData("BoundingBox")
{
    m_vbo = new VertexBuffer(VEC3_SIZE * CUBE_VERTICES_WIREFRAME.size(), CUBE_VERTICES_WIREFRAME.data());
    m_ebo = new ElementBuffer(UINT_SIZE * CUBE_INDICES_WIREFRAME.size(), CUBE_INDICES_WIREFRAME.data());

    m_vao->set_attrib_pointer(0, 3, GL_FLOAT, GL_FALSE, VEC3_SIZE, nullptr);
    m_vao->unbind();

    m_index_count = 24;
    m_draw_mode = GL_LINES;
}
