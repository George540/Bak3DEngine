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
	// RENDERING
	glm::vec3 m_ambient = glm::vec3(0.3f);
	glm::vec3 m_diffuse = glm::vec3(1.0f);
	glm::vec3 m_specular = glm::vec3(0.3f);
	float m_intensity = 1.0f;
	LightType m_type = LightType::Point;

	// DIRECTIONAL
	glm::vec3 m_direction;

	// POINT
	float m_attenuation_radius = 32.0f;

	// SPOT
	float m_inner_angle = 12.5f;
	float m_outer_angle = 17.5f;
	float m_inner_cut_off = glm::cos(glm::radians(m_inner_angle)); // glm::cos(glm::radians(inner_degrees))
	float m_outer_cut_off = glm::cos(glm::radians(m_outer_angle)); // glm::cos(glm::radians(outer_degrees))
	float m_cone_size = 1.0f;

	size_t m_ssbo_index = 0;

	TextureRef m_sprite_texture;
public:
	Light(LightType type = LightType::Point, glm::vec3 position = glm::vec3(0.0f)) ;
	~Light() override = default;

	void update(float dt) override;
	void draw() const override;

	// Type
	LightType get_type() const { return m_type; }
	void set_type(const LightType type);

	// Colors
	glm::vec3 get_ambient() const { return m_ambient; }
	glm::vec3 get_diffuse() const { return m_diffuse; }
	glm::vec3 get_specular() const { return m_specular;}
	float get_intensity() const { return m_intensity; }

	void set_ambient (glm::vec3 ambient);
	void set_diffuse (glm::vec3 diffuse);
	void set_specular(glm::vec3 specular);
	void set_intensity(float intensity);

	// Direction (directional + spot)
	glm::vec3 get_direction() const { return m_direction; }
	void set_direction(const glm::vec3 direction);

	float get_attenuation_radius() const { return m_attenuation_radius; }
	float get_cone_angle_inner_cutoff() const;
	float get_cone_angle_outer_cutoff() const;
	float get_cone_size() const { return m_cone_size; }

	// Attenuation (point + spot)
	void set_attenuation(float radius);
	// Cone angles in degrees — stored internally as cosines (spot only)
	void set_cone_angles(float inner_degrees, float outer_degrees);
	void set_cone_size(float size);

	LightDataPayload get_light_data_payload() const;
	size_t get_ssbo_index() const { return m_ssbo_index; }
private:
	void set_texture_by_type(LightType type);
};
