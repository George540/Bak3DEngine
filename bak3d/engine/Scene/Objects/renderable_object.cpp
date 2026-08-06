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

#include <filesystem>
#include <utility>

#include "renderable_object.h"

#include "Scene/scene.h"

using namespace std;

RenderableObject::RenderableObject(const MaterialRef& material, const glm::vec3 position, const std::string& name)
	: SceneObject(position, name)
{
	m_material_slot = make_material_slot(material);
	m_visible = true;

	m_vao = new VertexArray();
	m_vao->bind();

	// Will be handled by derived class
	m_vbo = nullptr;
	m_ebo = nullptr;
}

RenderableObject::~RenderableObject()
{
	delete m_vao;
	delete m_vbo;
	delete m_ebo;
}

void RenderableObject::update(float dt)
{
	set_model_matrix(m_position, m_scaling, m_euler_rotation, 0.0f);
}

void RenderableObject::draw() const
{
	Camera* scene_camera = Scene::instance->get_camera();
	if (!m_material_slot || !*m_material_slot || !scene_camera) return;

	(*m_material_slot)->set_mat4("model", m_model_matrix);
	apply_material();
}

InstancedRenderableObject::InstancedRenderableObject(const MaterialRef& material, const std::string& name) : RenderableObject(material, glm::vec3(0.0f, 0.0f, 0.0f), name), m_ibo(nullptr) {}

InstancedRenderableObject::~InstancedRenderableObject()
{
	delete m_ibo;
}

void InstancedRenderableObject::draw() const
{
	Camera* scene_camera = Scene::instance->get_camera();
	if (!m_material_slot || !*m_material_slot || !scene_camera) return;

	(*m_material_slot)->apply();
}
