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

#include <glm/vec3.hpp>
#include <glm/gtc/matrix_access.hpp>

#include "Renderer/Buffers/data_buffer.h"
#include "Scene/scene_object.h"

/*
 * Camera Frustum structure for culling object based on their bounding volumes or colliders
 */
struct Frustum
{
	enum PlaneSide : uint8_t { Left, Right, Bottom, Top, Near, Far, Count };

	Frustum() = default;
	explicit Frustum(const glm::mat4& view_projection)
	{
		const glm::vec4 row0 = glm::row(view_projection, 0);
		const glm::vec4 row1 = glm::row(view_projection, 1);
		const glm::vec4 row2 = glm::row(view_projection, 2);
		const glm::vec4 row3 = glm::row(view_projection, 3);

		planes[Left] = row3 + row0;
		planes[Right] = row3 - row0;
		planes[Bottom] = row3 + row1;
		planes[Top] = row3 - row1;
		planes[Near] = row3 + row2; // OpenGL clip space (z in -1..1)
		planes[Far] = row3 - row2;

		for (glm::vec4& plane : planes)
		{
			if (const float length = glm::length(glm::vec3(plane)); length > 0.0f)
			{
				plane /= length;
			}
		}
	}

	std::array<glm::vec4, Count> planes;
};

/*
 * Camera class that contains every information for moving the viewport in world space.
 * Contains other camera parameters.
 */
class Camera : public SceneObject
{
public:
	Camera(glm::vec3 position);
	~Camera() override = default;

	void update(float dt) override;

	[[nodiscard]] glm::mat4 get_view_matrix() const;
	[[nodiscard]] glm::mat4 get_projection_matrix() const;
	[[nodiscard]] glm::mat4 get_view_projection_matrix() const;
	[[nodiscard]] glm::vec3 get_camera_position() const { return transform.get_local_position(); }
	[[nodiscard]] UniformBuffer* get_camera_data_buffer() const { return m_camera_data_ubo.get(); }

	[[nodiscard]] glm::vec3 get_forward_vector() const;
	[[nodiscard]] glm::vec3 get_right_vector() const;
	[[nodiscard]] Frustum get_frustum() const { return m_frustum; }

private:
	void upload_camera_data() const;

	double m_cam_speed = 20.0f;
	float m_fov = DEFAULT_CAMERA_FOV; // camera zoom

	Frustum m_frustum = Frustum();

	std::unique_ptr<UniformBuffer> m_camera_data_ubo;
};