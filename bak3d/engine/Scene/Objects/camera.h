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
	static constexpr float DEFAULT_FOV = 30.0f;
	static constexpr float DEFAULT_NEAR_PLANE = 0.1f;
	static constexpr float DEFAULT_FAR_PLANE = 100.0f;
	static constexpr float DEFAULT_ASPECT_RATIO = 16.0f / 9.0f;
	static constexpr float MIN_FOV = 1.0f;
	static constexpr float MAX_FOV = 170.0f;

	Camera(glm::vec3 position, bool is_editor_camera = false);
	~Camera() override = default;

	void update(float dt) override;
	void draw_debug_geometry() const; // frustum, mirrors

	[[nodiscard]] glm::mat4 get_view_matrix() const;
	[[nodiscard]] glm::mat4 get_projection_matrix() const;
	[[nodiscard]] glm::mat4 get_view_projection_matrix() const;
	[[nodiscard]] glm::vec3 get_camera_position() const { return transform.get_local_position(); }
	[[nodiscard]] UniformBuffer* get_camera_data_buffer() const { return m_camera_data_ubo.get(); }
	[[nodiscard]] glm::vec3 get_forward_vector() const;
	[[nodiscard]] glm::vec3 get_right_vector() const;
	[[nodiscard]] Frustum get_frustum() const { return m_frustum; }

	[[nodiscard]] float get_fov() const { return m_fov; }
	[[nodiscard]] float get_near_plane() const { return m_near_plane; }
	[[nodiscard]] float get_far_plane() const { return m_far_plane; }
	[[nodiscard]] float get_aspect_ratio() const;
	void set_fov(const float fov) { m_fov = glm::clamp(fov, MIN_FOV, MAX_FOV); }
	void set_near_plane(const float near_plane) { m_near_plane = glm::clamp(near_plane, 0.001f, m_far_plane - 0.001f); }
	void set_far_plane(const float far_plane) { m_far_plane = glm::max(far_plane, m_near_plane + 0.001f); }
	void set_aspect_ratio(const float aspect) { m_aspect_ratio = glm::max(aspect, 0.1f); }

private:
	void update_editor_controls(float dt);
	void upload_camera_data() const;

	double m_cam_speed = 20.0f;
	float m_fov = DEFAULT_CAMERA_FOV;

	float m_near_plane = DEFAULT_NEAR_PLANE;
	float m_far_plane = DEFAULT_FAR_PLANE;
	float m_aspect_ratio = DEFAULT_ASPECT_RATIO;

	Frustum m_frustum = Frustum();
	std::unique_ptr<UniformBuffer> m_camera_data_ubo; // @TODO: Make camera UBO work properly between editor and game camera
};