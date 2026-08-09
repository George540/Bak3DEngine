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

#include "camera.h"

#include <glm/gtc/type_ptr.hpp>

#include "Core/logger.h"
#include "Input/event_manager.h"

using namespace std;

Camera::Camera(glm::vec3 position, glm::vec3 lookat, glm::vec3 up, float speed, float hor_angle, float ver_angle, float zoom) :
	SceneObject(position, "Camera"),
	m_lookat(lookat),
	m_cam_up(up),
	m_cam_speed(speed),
	m_horizontal_angle(hor_angle),
	m_vertical_angle(ver_angle),
	m_zoom(zoom)
{
	// @TODO: Replace with struct payload instead of manual size
	// 2 mat4's * 1 vec4 = 9 vec4's
	m_camera_data_ubo = make_unique<UniformBuffer>(VEC4_SIZE * 9 /*Temporary size*/, nullptr, 0, GL_DYNAMIC_DRAW);

	B3D_LOG_INFO("Camera has been set up.");
};

void Camera::update(float dt)
{
	// Prevent from having the camera move only when the cursor is within the windows
	EventManager::enable_mouse_cursor();

	// Mouse motion to get the variation in angle
	m_horizontal_angle -= EventManager::get_mouse_motion_x() * m_cam_speed * static_cast<double>(dt);
	m_vertical_angle -= EventManager::get_mouse_motion_y() * m_cam_speed * static_cast<double>(dt);

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

	const auto theta = static_cast<float>(glm::radians(m_horizontal_angle));
	const auto phi = static_cast<float>(glm::radians(m_vertical_angle));

	// Set position of camera while orbiting lookat
	m_position = glm::vec3(cosf(phi) * cosf(theta), sinf(phi), -cosf(phi) * sinf(theta));
	// Set zoom based on camera scroll offset
	m_position *= EventManager::get_camera_scroll_offset();

	m_camera_data_ubo->bind();
	m_camera_data_ubo->bind_buffer_sub_data(glm::value_ptr(get_projection_matrix()), MAT4_SIZE, 0);
	m_camera_data_ubo->bind_buffer_sub_data(glm::value_ptr(get_view_matrix()), MAT4_SIZE, MAT4_SIZE);
	m_camera_data_ubo->bind_buffer_sub_data(glm::value_ptr(glm::vec4(m_position, 1.0f)), VEC4_SIZE, 2 * MAT4_SIZE);
	m_camera_data_ubo->unbind();
}

glm::mat4 Camera::get_view_matrix() const
{
	return glm::lookAt(m_position, m_lookat, m_cam_up);
}

glm::mat4 Camera::get_projection_matrix() const
{
	return glm::perspective(glm::radians(m_zoom), static_cast<float>(EventManager::get_window_width()) / static_cast<float>(EventManager::get_window_height()), 0.1f, 100.0f);
}

glm::mat4 Camera::get_view_projection_matrix() const
{
	return get_projection_matrix() * get_view_matrix();
}