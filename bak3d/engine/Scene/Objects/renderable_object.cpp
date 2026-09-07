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

#include <glm/gtc/packing.hpp>

#include "Asset/mesh_data.h"
#include "Asset/resource_manager.h"
#include "Scene/scene.h"
#include "Scene/scene_manager.h"

using namespace std;

RenderableObject::RenderableObject(const MaterialRef& material, const glm::vec3 position, const std::string& name)
	: SceneObject(position, name)
{
	// Make sure that the default material is used as a fallback
	m_material_slot = make_material_slot(material ? material : ResourceManager::get_material("default_material"));
	m_mesh_slot = make_mesh_slot();

	update_self_and_children();
}

void RenderableObject::update(float dt)
{
	SceneObject::update(dt);
}

void RenderableObject::draw() const
{
	Camera* scene_camera = SceneManager::get_current_scene()->get_current_camera();
	if (!m_material_slot || !*m_material_slot || !scene_camera || !m_is_visible || !has_mesh()) return;

	(*m_material_slot)->bind_textures_cache();
	(*m_material_slot)->set_mat4("model", transform.get_global_model_matrix());
	(*m_material_slot)->set_int("active_light_count", SceneManager::get_current_scene()->get_all_lights().size());
	apply_material();

	(*m_mesh_slot)->draw();
}

void InstancedRenderableObject::ensure_capacity()
{
	const int needed = static_cast<int>(m_instances.size());
	if (needed <= m_capacity)
	{
		return;
	}

	m_instance_buffer = make_unique<InstanceBuffer>(INSTANCE_GPU_DATA_SIZE * needed, nullptr, GL_DYNAMIC_DRAW);

	m_instanced_vao = make_unique<VertexArray>();
	m_instanced_vao->bind();

	(*m_mesh_slot)->bind_vertex_attributes();

	// Mesh uses 0-6, so re-assign from 7 and after
	m_instance_buffer->bind();
	m_instanced_vao->set_attrib_pointer(7, 4, GL_FLOAT, GL_FALSE, INSTANCE_GPU_DATA_SIZE, nullptr, 1);
	m_instanced_vao->set_attrib_pointer(8, 2, GL_HALF_FLOAT, GL_FALSE, INSTANCE_GPU_DATA_SIZE, reinterpret_cast<void*>(offsetof(InstanceGPUData, rotation_half)), 1);
	m_instanced_vao->set_attrib_pointer(9, 4, GL_UNSIGNED_BYTE, GL_TRUE, INSTANCE_GPU_DATA_SIZE, reinterpret_cast<void*>(offsetof(InstanceGPUData, color_packed)), 1);

	m_instanced_vao->unbind();
	m_capacity = needed;
}

void InstancedRenderableObject::upload_instance_data() const
{
	m_instance_buffer->bind();
	m_instance_buffer->bind_buffer_sub_data(m_instances.data(), INSTANCE_GPU_DATA_SIZE * m_instances.size(), 0);
	m_is_dirty = false;
}

InstancedRenderableObject::InstancedRenderableObject(const MaterialRef& material, const glm::vec3 position, const string& name)
	: RenderableObject(material, position, name) {}

InstancedRenderableObject::~InstancedRenderableObject()
{
	m_instances.clear();
}

void InstancedRenderableObject::draw() const
{
	Camera* scene_camera = SceneManager::get_current_scene()->get_current_camera();
	if (!m_material_slot || !*m_material_slot || !scene_camera || !m_is_visible) return;

	(*m_material_slot)->apply();

	if (m_is_dirty)
	{
		const_cast<InstancedRenderableObject*>(this)->ensure_capacity();
		upload_instance_data();
	}

	(*m_material_slot)->bind_textures_cache();
	(*m_material_slot)->set_mat4("model", transform.get_global_model_matrix()); // batch root offset only
	apply_material();

	m_instanced_vao->bind();
	glDrawElementsInstanced(GL_TRIANGLES, (*m_mesh_slot)->get_index_count(), GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(m_instances.size()));
	m_instanced_vao->unbind();
}

int InstancedRenderableObject::add_instance(const glm::vec3& position, const glm::quat& rotation, float scale, const glm::vec4& color)
{
	InstanceGPUData instance {};

	instance.position = position;
	instance.scale = scale;

	// Pack color: vec4 -> uint32_t RGBA8
	instance.color_packed = glm::packUnorm4x8(color);

	// Pack quaternion: glm::quat -> vec2 half-precision
	// For normalized quaternion, drop Z and W, store X and Y only.
	// Store as 16-bit half-precision floats for preserving signs and accuracy.
	instance.rotation_half.x = glm::packHalf1x16(rotation.x);
	instance.rotation_half.y = glm::packHalf1x16(rotation.y);

	m_instances.push_back(instance);

	m_is_dirty = true;
	return static_cast<int>(m_instances.size()) - 1;
}

void InstancedRenderableObject::remove_instance(int index)
{
	if (index < 0 || index >= static_cast<int>(m_instances.size()))
	{
		return;
	}
	m_instances.erase(m_instances.begin() + index);
	m_is_dirty = true;
}

void InstancedRenderableObject::set_instance_data(int index, const glm::vec3& position, const glm::quat& rotation, const float scale, const glm::vec4& color)
{
	if (index < 0 || index >= static_cast<int>(m_instances.size()))
	{
		return;
	}

	m_instances[index].position = position;
	m_instances[index].scale = scale;
	m_instances[index].color_packed = glm::packUnorm4x8(color);
	m_instances[index].rotation_half.x = glm::packHalf1x16(rotation.x);
	m_instances[index].rotation_half.y = glm::packHalf1x16(rotation.y);

	m_is_dirty = true;
}
