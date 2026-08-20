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

#include "mesh.h"

#include <iostream>
#include <utility>

#include "Asset/mesh_data.h"
#include "Asset/resource_manager.h"
#include "Asset/texture.h"

using namespace std;

Mesh::Mesh(vector<Vertex> vertices, vector<GLuint> indices, const std::string& name) :
    RenderableObject(nullptr, glm::vec3(0.0f, 0.0f, 0.0f), name), // no initial material and position at first
	m_vertices(std::move(vertices)),
	m_indices(std::move(indices))
{

}

void Mesh::update(float dt)
{
    RenderableObject::update(dt);
}

void Mesh::draw() const
{
    RenderableObject::draw();

    (*m_mesh_slot)->get_vao()->bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, nullptr);
    (*m_mesh_slot)->get_vao()->unbind();
}
