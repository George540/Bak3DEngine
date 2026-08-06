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

#include "bounding_box.h"

#include "Asset/resource_manager.h"
#include "Core/global_definitions.h"
#include "Scene/scene.h"

BoundingBox::BoundingBox() : RenderableObject(ResourceManager::get_material("grid"), glm::vec3(0.0f), "Bounding Box")
{
    m_vao->bind_buffer();
    m_vbo = new VertexBuffer(VEC3_SIZE * CUBE_VERTICES_WIREFRAME.size(), CUBE_VERTICES_WIREFRAME.data());
    m_ebo = new ElementBuffer(sizeof(GLuint) * CUBE_INDICES_WIREFRAME.size(), CUBE_INDICES_WIREFRAME.data());

    m_vao->set_attrib_pointer(0, 3, GL_FLOAT, GL_FALSE, VEC3_SIZE, nullptr);
    m_vao->unbind_buffer();
}

void BoundingBox::update(float dt)
{
    RenderableObject::update(dt);
}

void BoundingBox::draw() const
{
    glDepthFunc(GL_ALWAYS);

    (*m_material_slot)->set_vec4("color", glm::vec4(1.0f));
    RenderableObject::draw();

    m_vao->bind_buffer();
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, nullptr);
    m_vao->unbind_buffer();

    glDepthFunc(GL_LESS);
}

