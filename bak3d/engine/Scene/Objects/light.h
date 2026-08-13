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

#include "renderable_object.h"
#include "Renderer/Buffers/data_buffer.h"

enum class LightType : int
{
	Directional = 0,
	Point = 1,
	Spot = 2,
	Area = 3,
	Max = 4
};

inline const char* light_type_to_string(const LightType type)
{
	switch (type)
	{
		case LightType::Directional: return "Directional";
		case LightType::Point: return "Point";
		case LightType::Spot: return "Spot";
		case LightType::Area: return "Area";
		case LightType::Max: return "Max";
		default: return "unknown";
	}
}

/*
 * Basic Light class that draws a cube light source (will use sprite instead of cube in the future).
 */
class Light : public RenderableObject
{
protected:
	// ORIENTATION
	float m_horizontal_angle;
	float m_vertical_angle;
	float m_distance_offset;

	// RENDERING
	glm::vec3 m_ambient;
	glm::vec3 m_diffuse;
	glm::vec3 m_specular;
	float m_intensity;
	LightType m_type;

	// DIRECTIONAL
	glm::vec3 m_direction;

	// POINT
	float m_attenuation_radius;

	// SPOT
	float m_inner_cut_off; // glm::cos(glm::radians(inner_degrees))
	float m_outer_cut_off; // glm::cos(glm::radians(outer_degrees))
	float m_cone_size;

	std::unique_ptr<UniformBuffer> m_light_data_ubo;

	TextureRef m_sprite_texture;
public:
	Light(glm::vec3 position, glm::vec3 scaling, const MaterialRef& material);
	~Light() override = default;

	void update(float dt) override;
	void draw() const override;

	UniformBuffer* get_camera_data_ubo() const { return m_light_data_ubo.get(); }

	float get_horizontal_angle() const { return m_horizontal_angle; }
	void set_horizontal_angle(const float angle) { m_horizontal_angle = angle; }

	float get_vertical_angle() const { return m_horizontal_angle; }
	void set_vertical_angle(const float angle) { m_vertical_angle = angle; }

	float get_distance_offset() const { return m_distance_offset; }
	void set_distance_offset(const float distance) { m_distance_offset = distance; }

	// Type
	LightType get_type() const { return m_type; }
	void set_type(const LightType type) { m_type = type; }

	// Colors
	glm::vec3 get_ambient() const { return m_ambient; }
	glm::vec3 get_diffuse() const { return m_diffuse; }
	glm::vec3 get_specular() const { return m_specular;}
	float get_intensity() const { return m_intensity; }

	void set_ambient (const glm::vec3 ambient) { m_ambient = ambient; }
	void set_diffuse (const glm::vec3 diffuse) { m_diffuse = diffuse; }
	void set_specular(const glm::vec3 specular) { m_specular = specular; }
	void set_intensity(const float intensity) { m_intensity = intensity; }

	// Direction (directional + spot)
	glm::vec3 get_direction() const { return m_direction; }
	void set_direction(const glm::vec3 direction) { m_direction = glm::normalize(direction); }

	float get_attenuation_radius() const { return m_attenuation_radius; }
	float get_cone_angle_inner_cutoff() const { return m_inner_cut_off; }
	float get_cone_angle_outer_cutoff() const { return m_outer_cut_off; }
	float get_cone_size() const { return m_cone_size; }

	// Attenuation (point + spot)
	void set_attenuation(const float radius);
	// Cone angles in degrees — stored internally as cosines (spot only)
	void set_cone_angles(float inner_degrees, float outer_degrees);
	void set_cone_size(const float size) { m_cone_size = size; };
private:
	void update_light_data_ubo() const;
	void set_texture_by_type(LightType type);
};