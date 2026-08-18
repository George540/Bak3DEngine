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

#include <glm/fwd.hpp>
#include <glm/vec3.hpp>

#include "Renderer/Buffers/data_buffer.h"
#include "Scene/scene_object.h"

/*
 * Camera class that contains every information for moving the viewport in world space.
 * Contains other camera parameters.
 */
class Camera : public SceneObject
{
public:
	Camera(glm::vec3 position,
		glm::vec3 lookat,
		glm::vec3 up, 
		float speed, 
		float hor_angle, 
		float ver_angle, 
		float zoom);
	~Camera() override = default;

	void update(float dt) override;

	[[nodiscard]] glm::mat4 get_view_matrix() const;
	[[nodiscard]] glm::mat4 get_projection_matrix() const;
	[[nodiscard]] glm::mat4 get_view_projection_matrix() const;
	[[nodiscard]] glm::vec3 get_camera_position() const { return transform.get_global_position(); }
	[[nodiscard]] UniformBuffer* get_camera_data_buffer() const { return m_camera_data_ubo.get(); }
private:
	glm::vec3 m_lookat; // look towards this point
	glm::vec3 m_cam_up; // up vector

	double m_cam_speed;
	double m_horizontal_angle; // horizontal angle
	double m_vertical_angle;   // vertical angle
	float m_zoom; // camera zoom

	std::unique_ptr<UniformBuffer> m_camera_data_ubo;
};