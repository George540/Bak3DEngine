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

#include <iostream>

#include "light.h"

#include "Core/global_definitions.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/epsilon.hpp>

#include "Asset/resource_manager.h"
#include "Asset/texture.h"
#include "Core/global_settings.h"
#include "Core/logger.h"
#include "Renderer/renderer.h"
#include "Scene/scene.h"
#include "Scene/scene_manager.h"

Light::Light(const LightType type, const glm::vec3 position) :
	RenderableObject(ResourceManager::get_material("light_icon"), position, "Light")
{
	m_type = type;
	object_type = SceneObjectType::Light;

	m_ssbo_index = 0;

	m_mesh_slot = make_mesh_slot(ResourceManager::get_mesh("Quad"));
	set_texture_by_type(m_type);

	B3D_LOG_INFO("Light created.");
}

void Light::update(float dt)
{
	RenderableObject::update(dt);
}

void Light::draw() const
{
	(*m_material_slot)->set_vec4("diffuseColor", glm::vec4(m_diffuse, 1.0f));

	m_sprite_texture->bind(0);
	
	RenderableObject::draw();

	Texture2D::unbind();
}

void Light::set_type(const LightType type)
{
	if (m_type == type)
	{
		return;
	}

	m_type = type;
	set_texture_by_type(type);
	m_is_dirty = true;
}

void Light::set_ambient(const glm::vec3 ambient)
{
	if (glm::all(glm::epsilonEqual(m_ambient, ambient, EPSILON_CUSTOM)))
	{
		return;
	}

	m_ambient = ambient;
	m_is_dirty = true;
}

void Light::set_diffuse(const glm::vec3 diffuse)
{
	if (glm::all(glm::epsilonEqual(m_diffuse, diffuse, EPSILON_CUSTOM)))
	{
		return;
	}

	m_diffuse = diffuse;
	m_is_dirty = true;
}

void Light::set_specular(const glm::vec3 specular)
{
	if (glm::all(glm::epsilonEqual(m_specular, specular, EPSILON_CUSTOM)))
	{
		return;
	}

	m_specular = specular;
	m_is_dirty = true;
}

void Light::set_intensity(const float intensity)
{
	if (glm::epsilonEqual(m_intensity, intensity, EPSILON_CUSTOM))
	{
		return;
	}

	m_intensity = intensity;
	m_is_dirty = true;
}

void Light::set_direction(const glm::vec3 direction)
{
	const glm::vec3 normalized_direction = glm::normalize(direction);
	if (glm::all(glm::epsilonEqual(m_direction, normalized_direction, EPSILON_CUSTOM)))
	{
		return;
	}

	m_direction = normalized_direction;
	m_is_dirty = true;
}

float Light::get_cone_angle_inner_cutoff() const
{
	return m_inner_angle;
}

float Light::get_cone_angle_outer_cutoff() const
{
	return m_outer_angle;
}

void Light::set_attenuation(const float radius)
{
	if (glm::epsilonEqual(m_attenuation_radius, radius, EPSILON_CUSTOM))
	{
		return;
	}

	m_attenuation_radius = radius;
	m_is_dirty = true;
}

void Light::set_cone_angles(float inner_degrees, float outer_degrees)
{
	if (glm::epsilonEqual(m_inner_angle, inner_degrees, EPSILON_CUSTOM)
		&& glm::epsilonEqual(m_outer_angle, outer_degrees, EPSILON_CUSTOM))
	{
		return;
	}

	m_inner_angle = inner_degrees;
	m_outer_angle = outer_degrees;

	m_inner_cut_off = glm::cos(glm::radians(m_inner_angle + m_cone_size));
	m_outer_cut_off = glm::cos(glm::radians(m_outer_angle + m_cone_size));

	m_is_dirty = true;
}

void Light::set_cone_size(const float size)
{
	if (glm::epsilonEqual(m_cone_size, size, EPSILON_CUSTOM))
	{
		return;
	}

	m_cone_size = size;

	m_is_dirty = true;
}

LightDataPayload Light::get_light_data_payload() const
{
	LightDataPayload payload {};
	payload.position = glm::vec4(transform.get_global_position(), m_inner_cut_off);
	payload.direction = glm::vec4(m_direction, m_outer_cut_off);
	payload.ambient = glm::vec4(m_ambient, m_attenuation_radius);
	payload.diffuse = glm::vec4(m_diffuse, m_intensity);
	payload.specular = glm::vec4(m_specular, 0.0f);
	payload.type = static_cast<int32_t>(m_type);
	return payload;
}

void Light::set_texture_by_type(const LightType type)
{
	switch (type)
	{
		case LightType::Directional: m_sprite_texture = ResourceManager::get_texture("directional_light_icon.png"); break;
		case LightType::Point: m_sprite_texture = ResourceManager::get_texture("point_light_icon.png"); break;
		case LightType::Spot: m_sprite_texture = ResourceManager::get_texture("spot_light_icon.png"); break;
		case LightType::Area: m_sprite_texture = ResourceManager::get_texture("area_light_icon.png"); break;
		default: m_sprite_texture = ResourceManager::get_texture("point_light_icon.png"); break;
	}
}
