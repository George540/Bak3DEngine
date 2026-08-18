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
#include <vector>
#include <filesystem>

#include <glm/ext.hpp>

#include "grid.h"

#include "Core/logger.h"

Grid::Grid(MaterialRef material)
	: RenderableObject(material, glm::vec3(0.0f, 0.0f, 0.0f), "Grid")
	, m_number_of_slices(40)
	, m_grid_size(20.0f)
{
	// GRID LINE SETUP
	// Grid setup used indices and EBOs for proper identification of each line
	// Part of the solution is referenced from thewoz's answer on stackoverflow.com
	// Link to question and answer: https://stackoverflow.com/questions/58494179/how-to-create-a-grid-in-opengl-and-drawing-it-with-lines#:~:text=this%20the%20code%20for%20render,opengl
	std::vector<glm::vec3> vertices;
	std::vector<glm::uvec4> indices;

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

	m_vbo = new VertexBuffer(VEC3_SIZE * vertices.size(), &vertices[0]);
	m_ebo = new ElementBuffer(UVEC4_SIZE * indices.size(), &indices[0]);

	m_vao->set_attrib_pointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	m_vao->set_attrib_pointer(1, 3, GL_FLOAT, GL_FALSE, 2 * VEC3_SIZE, (void*)VEC3_SIZE);
	m_vao->unbind();

	m_number_of_elements = static_cast<GLuint>(indices.size()) * 4;

	update_self_and_children();

	B3D_LOG_INFO("Setting up grid...");
}

void Grid::draw() const
{
	(*m_material_slot)->set_vec4("color", glm::vec4(0.5f));
	RenderableObject::draw();

	m_vao->bind();
	glDrawElements(GL_LINES, m_number_of_elements, GL_UNSIGNED_INT, nullptr);
	m_vao->unbind();
}