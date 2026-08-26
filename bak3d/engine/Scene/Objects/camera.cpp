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

#include <glm/gtc/type_ptr.hpp>

#include "Core/logger.h"
#include "Input/event_manager.h"

using namespace std;

constexpr static glm::vec3 CAMERA_UP = glm::vec3(0.0f, 1.0f, 0.0f);

Camera::Camera(glm::vec3 position) : SceneObject(position, "Camera")
{
	object_type = SceneObjectType::Camera;

	// @TODO: Replace with struct payload instead of manual size
	// 2 mat4's * 1 vec4 = 9 vec4's
	m_camera_data_ubo = make_unique<UniformBuffer>(VEC4_SIZE * 9 /*Temporary size*/, nullptr, 0, GL_DYNAMIC_DRAW);

	B3D_LOG_INFO("Camera has been set up.");
};

void Camera::update(float dt)
{
	// Prevent from having the camera move only when the cursor is within the windows
	EventManager::enable_mouse_cursor();

	if (EventManager::is_camera_looking())
	{
		// Mouse Look
		m_horizontal_angle -= EventManager::get_mouse_motion_x() * m_cam_speed * dt;
		m_vertical_angle -= EventManager::get_mouse_motion_y() * m_cam_speed * dt;

		// Prevent camera from flipping upside down
		// Clamp vertical angle to [-85, 85] degrees
		m_vertical_angle = max(-85.0f, min(85.0f, static_cast<float>(m_vertical_angle)));
		if (m_horizontal_angle > 360)
		{
			m_horizontal_angle -= 360;
		}
		else if (m_horizontal_angle < -360)
		{
			m_horizontal_angle += 360;
		}

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

	// FOV handling and clamping
	if (const double scroll_delta = EventManager::get_camera_scroll_offset(); scroll_delta != 0.0)
	{
		m_fov -= static_cast<float>(scroll_delta) * 2.0f;
		m_fov = clamp(m_fov, 20.0f, 90.0f);
	}

	m_camera_data_ubo->bind();
	m_camera_data_ubo->bind_buffer_sub_data(glm::value_ptr(get_projection_matrix()), MAT4_SIZE, 0);
	m_camera_data_ubo->bind_buffer_sub_data(glm::value_ptr(get_view_matrix()), MAT4_SIZE, MAT4_SIZE);
	m_camera_data_ubo->bind_buffer_sub_data(glm::value_ptr(glm::vec4(transform.get_local_position(), 1.0f)), VEC4_SIZE, 2 * MAT4_SIZE);
	m_camera_data_ubo->unbind();

	SceneObject::update(dt);
}

glm::mat4 Camera::get_view_matrix() const
{
	const glm::vec3 position = transform.get_local_position();
	const glm::vec3 forward = get_forward_vector();

	return glm::lookAt(position, position + forward, CAMERA_UP);
}

glm::mat4 Camera::get_projection_matrix() const
{
	const float aspect = static_cast<float>(EventManager::get_viewport_width()) / static_cast<float>(EventManager::get_viewport_height());
	return glm::perspective(glm::radians(m_fov), aspect, 0.1f, 100.0f);
}

glm::mat4 Camera::get_view_projection_matrix() const
{
	return get_projection_matrix() * get_view_matrix();
}

glm::vec3 Camera::get_forward_vector() const
{
	const float yaw = glm::radians(static_cast<float>(m_horizontal_angle));
	const float pitch = glm::radians(static_cast<float>(m_vertical_angle));

	glm::vec3 forward;
	forward.x = cosf(pitch) * cosf(yaw);
	forward.y = sinf(pitch);
	forward.z = -cosf(pitch) * sinf(yaw);

	return glm::normalize(forward);
}

glm::vec3 Camera::get_right_vector() const
{
	const glm::vec3 forward = get_forward_vector();
	return glm::normalize(glm::cross(forward, CAMERA_UP));
}