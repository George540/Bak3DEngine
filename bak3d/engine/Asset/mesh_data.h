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

#include "asset.h"
#include "Core/global_definitions.h"
#include "Renderer/Buffers/buffer.h"
#include "Renderer/Buffers/vertex_array.h"

/*
 * Mesh asset that holds rendering data for rendering a mesh in the scene.
 * NOTE: This is not an asset on disk, but a transient read-only version
 *       of a unique part of a model found inside a model asset.
 */
class PrimitiveData : public Asset
{
public:
    PrimitiveData() = default;
    PrimitiveData(const std::string& name);
    ~PrimitiveData() override = default;

    void draw() const
    {
        m_vao->bind();
        glDrawElements(m_draw_mode, m_index_count, GL_UNSIGNED_INT, nullptr);
        m_vao->unbind();
    }

    void draw_instanced(const GLsizei instance_count) const
    {
        m_vao->bind();
        glDrawElementsInstanced(m_draw_mode, m_index_count, GL_UNSIGNED_INT, nullptr, instance_count);
        m_vao->unbind();
    }

    const VertexArray* get_vao() const { return m_vao; }
    const VertexBuffer* get_vbo() const { return m_vbo; }
    const ElementBuffer* get_ebo() const { return m_ebo; }

protected:
    VertexArray* m_vao = nullptr;
    VertexBuffer* m_vbo = nullptr;
    ElementBuffer* m_ebo = nullptr;

    GLsizei m_index_count = 0;
    GLenum m_draw_mode = GL_TRIANGLES;
};

class MeshData : public PrimitiveData
{
public:
    MeshData() = default;
    MeshData(std::vector<Vertex> vertices, std::vector<GLuint> indices, const std::string& name);
    ~MeshData() override = default;

    const std::vector<Vertex>& get_vertices() const { return m_vertices; }
    const std::vector<GLuint>& get_indices() const { return m_indices; }

private:
    std::vector<Vertex> m_vertices;
    std::vector<GLuint> m_indices;
};

class QuadData : public PrimitiveData
{
public:
    QuadData();
    ~QuadData() override = default;
};

class GridData : public PrimitiveData
{
public:
    GridData();
    GridData(int num_of_elements, float grid_size);
    ~GridData() override = default;
private:
    int m_number_of_slices;
    float m_grid_size;
};

class BoundingBoxData : public PrimitiveData
{
public:
    BoundingBoxData();
    ~BoundingBoxData() override = default;
};
