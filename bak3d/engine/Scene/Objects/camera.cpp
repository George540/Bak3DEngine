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

#include <algorithm>
#include <iostream>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>

#include "camera.h"

#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Asset/mesh_data.h"
#include "Asset/resource_manager.h"
#include "Core/logger.h"
#include "Input/event_manager.h"
#include "Scene/scene_manager.h"

using namespace std;

constexpr static glm::vec3 CAMERA_UP = glm::vec3(0.0f, 1.0f, 0.0f);

namespace
{
	constexpr glm::vec4 CAMERA_DEBUG_COLOR = glm::vec4(0.78f, 0.55f, 1.0f, 1.0f);
	int m_camera_num = 0;
}

Camera::Camera(glm::vec3 position, bool is_editor_camera)
	: RenderableObject(ResourceManager::get_material("light_icon"), position, is_editor_camera ? "EditorCamera" : "Camera", is_editor_camera)
{
	object_type = SceneObjectType::Camera;
	transform.set_local_euler_rotation(glm::vec3(-20.0f, 45.0f, 0.0f));

	m_mesh_slot = make_mesh_slot(ResourceManager::get_mesh("Quad"));
	m_sprite_texture = ResourceManager::get_texture("camera_icon.png");

	m_camera_id = m_camera_num++;

	// @TODO: Replace with struct payload instead of manual size
	// 2 mat4's * 1 vec4 = 9 vec4's
	m_camera_data_ubo = make_unique<UniformBuffer>(VEC4_SIZE * 9, nullptr, 0, GL_DYNAMIC_DRAW);

	B3D_LOG_INFO("%s has been set up.", get_object_name().c_str());
};

void Camera::update(float dt)
{
	// Only editor cameras touch input or the global camera UBO
	if (m_is_editor_only)
	{
		update_editor_controls(dt);

		// Editor camera planes are driven by global settings
		set_far_plane(GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::EditorSettings_FarPlane));
		set_near_plane(GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::EditorSettings_NearPlane));
	}

	m_frustum = Frustum(get_view_projection_matrix());
	upload_camera_data();

	RenderableObject::update(dt);
}

void Camera::draw() const
{
	if (m_is_editor_only || !m_sprite_texture)
	{
		return;
	}

	(*m_material_slot)->set_vec4("diffuseColor", glm::vec4(1.0f));
	(*m_material_slot)->set_float("icon_size_multiplier", GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::EditorOverlayFlags_LightIconSize));
	m_sprite_texture->bind(0);
	RenderableObject::draw();
	Texture2D::unbind();
}

void Camera::draw_debug_geometry() const
{
	const MaterialRef material = ResourceManager::get_material("line");
	const MeshRef mesh = ResourceManager::get_mesh("FrustumDebug");
	if (!material || !mesh)
	{
		return;
	}

	material->set_vec4("color", CAMERA_DEBUG_COLOR);

	const float tan_half_fov = glm::tan(glm::radians(m_fov * 0.5f));
	const float aspect = get_aspect_ratio();
	const glm::mat4 camera_to_world = glm::inverse(get_view_matrix());

	for (const float distance : { m_near_plane, m_far_plane })
	{
		const float half_height = distance * tan_half_fov;
		material->set_mat4("model", camera_to_world * glm::scale(glm::mat4(1.0f), glm::vec3(half_height * aspect, half_height, distance)));
		material->apply();
		mesh->draw();
	}
}

void Camera::update_editor_controls(float dt)
{
	EventManager::enable_mouse_cursor(); // keep original behaviour

	if (EventManager::is_camera_looking())
	{
		// Rotation
		const float mouse_x = EventManager::get_mouse_motion_x();
		const float mouse_y = EventManager::get_mouse_motion_y();

		glm::vec3 rotation = transform.get_local_euler_rotation();
		rotation.y -= mouse_x * EventManager::get_mouse_sensitivity(); // Yaw
		rotation.x += mouse_y * EventManager::get_mouse_sensitivity(); // Pitch
		rotation.x = glm::clamp(rotation.x, -85.0f, 85.0f); // Prevent camera from flipping upside down
		transform.set_local_euler_rotation(rotation);

		// Movement
		glm::vec3 movement(0.0f);

		const glm::vec3 forward = get_forward_vector();
		const glm::vec3 right = get_right_vector();

		if (EventManager::is_key_moving_forward_down())
		{
			movement += forward;
		}
		if (EventManager::is_key_moving_back_down())
		{
			movement -= forward;
		}
		if (EventManager::is_key_moving_right_down())
		{
			movement += right;
		}
		if (EventManager::is_key_moving_left_down())
		{
			movement -= right;
		}

		// Vertical movement
		if (EventManager::is_key_moving_up_down())
		{
			movement += CAMERA_UP;
		}
		if (EventManager::is_key_moving_down_down())
		{
			movement -= CAMERA_UP;
		}

		// Normalize so diagonal movement isn't faster
		if (glm::length2(movement) > 0.0f)
		{
			movement = glm::normalize(movement);
			transform.set_local_position(transform.get_local_position() + movement * static_cast<float>(m_cam_speed) * dt);
		}
	}

	if (const double scroll_delta = EventManager::get_camera_scroll_offset(); scroll_delta != 0.0)
	{
		m_fov -= static_cast<float>(scroll_delta) * 2.0f;
		m_fov = glm::clamp(m_fov, 20.0f, 90.0f);
	}
}

glm::mat4 Camera::get_view_matrix() const
{
	const glm::vec3 position = transform.get_local_position();
	const glm::vec3 forward = get_forward_vector();

	return glm::lookAt(position, position + forward, CAMERA_UP);
}

glm::mat4 Camera::get_projection_matrix() const
{
	return glm::perspective(glm::radians(m_fov), get_aspect_ratio(), m_near_plane, m_far_plane);
}

glm::mat4 Camera::get_view_projection_matrix() const
{
	return get_projection_matrix() * get_view_matrix();
}

glm::vec3 Camera::get_forward_vector() const
{
	const glm::mat4& model = transform.get_global_model_matrix();
	return glm::normalize(glm::vec3(model[2]) * -1.0f);
}

glm::vec3 Camera::get_right_vector() const
{
	const glm::mat4& model = transform.get_global_model_matrix();
	return glm::normalize(glm::vec3(model[0]));
}

float Camera::get_aspect_ratio() const
{
	// Editor: viewport aspect, Game: m_aspect_ratio
	if (m_is_editor_only)
	{
		return static_cast<float>(EventManager::get_viewport_width()) / static_cast<float>(EventManager::get_viewport_height());
	}
	return m_aspect_ratio;
}

void Camera::upload_camera_data() const
{
	if (!m_camera_data_ubo)
	{
		return;
	}

	m_camera_data_ubo->bind();
	m_camera_data_ubo->bind_buffer_sub_data(glm::value_ptr(get_projection_matrix()), MAT4_SIZE, 0);
	m_camera_data_ubo->bind_buffer_sub_data(glm::value_ptr(get_view_matrix()), MAT4_SIZE, MAT4_SIZE);
	m_camera_data_ubo->bind_buffer_sub_data(glm::value_ptr(glm::vec4(transform.get_local_position(), 1.0f)), VEC4_SIZE, 2 * MAT4_SIZE);
	m_camera_data_ubo->unbind();
	m_camera_data_ubo->bind_to_binding_point(m_camera_id);
}
