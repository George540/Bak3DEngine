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

#include <iostream>

#include "axis.h"

#include "Core/global_definitions.h"
#include "Core/logger.h"

Axis::Axis(MaterialRef material) :
	RenderableObject(material, glm::vec3(0.0f, 0.0f, 0.0f), "Axis")
{
	m_scaling = glm::vec3(3.0f);
	set_model_matrix(glm::vec3(0.0f), m_scaling, m_euler_rotation, 0.0f);

	m_vbo = new VertexBuffer(sizeof(AxisVertex) * AXIS_VERTICES.size(), AXIS_VERTICES.data());
	m_ebo = new ElementBuffer(sizeof(GLuint) * AXIS_INDICES.size(), AXIS_INDICES.data());

	m_vao->set_attrib_pointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(AxisVertex), reinterpret_cast<void*>(offsetof(AxisVertex, position)));
	m_vao->set_attrib_pointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(AxisVertex), reinterpret_cast<void*>(offsetof(AxisVertex, color)));
	m_vao->unbind_buffer();

	B3D_LOG_INFO("Setting up 3D axis gizmo...");
}

void Axis::draw() const
{
	glDepthFunc(GL_ALWAYS);

	RenderableObject::draw();

	m_vao->bind_buffer();
	glDrawElements(GL_LINES, static_cast<GLsizei>(AXIS_INDICES.size()), GL_UNSIGNED_INT, nullptr);
	m_vao->unbind_buffer();

	glDepthFunc(GL_LESS);
}