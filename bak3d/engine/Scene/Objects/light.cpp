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
#include "Input/event_manager.h"
#include "Renderer/renderer.h"
#include "Scene/scene.h"

namespace
{
	constexpr glm::vec4 LIGHT_DEBUG_COLOR = glm::vec4(0.55f, 0.78f, 1.0f, 1.0f);
	constexpr float DIRECTIONAL_DEBUG_LENGTH = 3.0f;
	constexpr float MAX_CONE_HALF_ANGLE = 89.0f; // tan() explodes at 90

	void draw_composite_debug_light_shape(const MaterialRef& material, const char* mesh_name, const glm::mat4& model)
	{
		const MeshRef mesh = ResourceManager::get_mesh(mesh_name);
		if (!mesh)
		{
			return;
		}
		material->set_mat4("model", model);
		material->apply();
		mesh->draw();
	}
}

Light::Light(const LightType type, const glm::vec3 position) :
	RenderableObject(ResourceManager::get_material("light_icon"), position, "Light")
{
	m_type = type;
	object_type = SceneObjectType::Light;

	transform.set_local_euler_rotation(glm::vec3(-45.0f));
	update_self_and_children();
	refresh_direction_from_transform();

	m_mesh_slot = make_mesh_slot(ResourceManager::get_mesh("Quad"));
	set_texture_by_type(m_type);

	B3D_LOG_INFO("Light created.");
}

void Light::update(float dt)
{
	RenderableObject::update(dt);
	refresh_direction_from_transform();
}

void Light::draw() const
{
	(*m_material_slot)->set_vec4("diffuseColor", glm::vec4(m_diffuse, 1.0f));
	(*m_material_slot)->set_float("icon_size_multiplier", GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::EditorOverlayFlags_LightIconSize));

	m_sprite_texture->bind(0);
	
	RenderableObject::draw();

	Texture2D::unbind();
}

void Light::draw_debug_geometry() const
{
	const MaterialRef material = ResourceManager::get_material("line");
	if (!material)
	{
		return;
	}
	material->set_vec4("color", LIGHT_DEBUG_COLOR);

	const glm::vec3 position = transform.get_global_position();
	const glm::vec3 direction = glm::normalize(m_direction);
	const glm::vec3 up = glm::abs(direction.y) > 0.99f ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
	const glm::mat4 oriented = glm::inverse(glm::lookAt(position, position + direction, up)); // local -Z

	switch (m_type)
	{
		case LightType::Point:
		{
			const glm::mat4 base = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), glm::vec3(m_attenuation_radius));
			draw_composite_debug_light_shape(material, "CircleDebug", base); // XY
			draw_composite_debug_light_shape(material, "CircleDebug", base * glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f))); // XZ
			draw_composite_debug_light_shape(material, "CircleDebug", base * glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f))); // YZ
			break;
		}
		case LightType::Directional:
			draw_composite_debug_light_shape(material, "LineDebug", oriented * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, DIRECTIONAL_DEBUG_LENGTH)));
			break;
		case LightType::Spot:
		case LightType::Area: // @TODO: Make unique geometry for Area lights
		{
			const float length = m_attenuation_radius;
			const float inner_half = glm::min(m_inner_angle + m_cone_size, MAX_CONE_HALF_ANGLE);
			const float outer_half = glm::min(m_outer_angle + m_cone_size, MAX_CONE_HALF_ANGLE);
			const float inner_radius = length * glm::tan(glm::radians(inner_half));
			const float outer_radius = length * glm::tan(glm::radians(outer_half));

			draw_composite_debug_light_shape(material, "ConeDebug", oriented * glm::scale(glm::mat4(1.0f), glm::vec3(inner_radius, inner_radius, length)));
			draw_composite_debug_light_shape(material, "ConeDebug", oriented * glm::scale(glm::mat4(1.0f), glm::vec3(outer_radius, outer_radius, length)));
			draw_composite_debug_light_shape(material, "LineDebug", oriented * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, length)));
			break;
		}
		default:
			break;
	}
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
	if (glm::dot(direction, direction) < 1e-12f)
	{
		return;
	}

	const glm::vec3 world_direction = glm::normalize(direction);
	glm::vec3 local_dir = world_direction;
	if (parent)
	{
		local_dir = glm::normalize(glm::mat3(glm::inverse(parent->transform.get_global_model_matrix())) * world_direction);
	}

	glm::vec3 euler = transform.get_local_euler_rotation();
	euler.x = glm::degrees(glm::asin(glm::clamp(local_dir.y, -1.0f, 1.0f)));
	if (glm::abs(local_dir.y) < 0.9999f)
	{
		euler.y = glm::degrees(glm::atan(-local_dir.x, -local_dir.z));
	}

	if (glm::all(glm::epsilonEqual(euler, transform.get_local_euler_rotation(), EPSILON_CUSTOM)))
	{
		return;
	}

	transform.set_local_euler_rotation(euler);
	m_direction = world_direction;
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

	recompute_cone_cutoffs();

	m_is_dirty = true;
}

void Light::set_cone_size(const float size)
{
	if (glm::epsilonEqual(m_cone_size, size, EPSILON_CUSTOM))
	{
		return;
	}

	m_cone_size = size;

	recompute_cone_cutoffs();

	m_is_dirty = true;
}

float Light::get_effective_radius() const
{
	if (m_type == LightType::Directional)
	{
		return numeric_limits<float>::max();
	}

	// Use the same attenuation model as in the shader.
	constexpr float ATTENUATION_CUTOFF_THRESHOLD = 1.0f / 256.0f;
	const float c = 1.0f;
	const float l = 4.5f / m_attenuation_radius;
	const float q = 75.0f / (m_attenuation_radius * m_attenuation_radius);
	const float target = m_intensity / ATTENUATION_CUTOFF_THRESHOLD;

	const float b_coefficient = l;
	const float c_coefficient = c - target;
	const float discriminant = b_coefficient * b_coefficient - 4.0f * q * c_coefficient;

	if (discriminant < 0.0f || q <= 0.0f)
	{
		return m_attenuation_radius; // Just in case, but it shouldn't trigger
	}

	return (-b_coefficient + glm::sqrt(discriminant)) / (2.0f * q);
}

LightGPUData Light::get_light_gpu_data_payload() const
{
	LightGPUData data {};
	data.position = glm::vec4(transform.get_global_position(), m_inner_cut_off);
	data.direction = glm::vec4(m_direction, m_outer_cut_off);
	data.ambient = glm::vec4(m_ambient, m_attenuation_radius);
	data.diffuse = glm::vec4(m_diffuse, m_intensity);
	data.specular = glm::vec4(m_specular, 0.0f);
	data.type = static_cast<int32_t>(m_type);
	data.shadow_index = 0; // cast shadows by default
	return data;
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

void Light::recompute_cone_cutoffs()
{
	m_inner_cut_off = glm::cos(glm::radians(m_inner_angle + m_cone_size));
	m_outer_cut_off = glm::cos(glm::radians(m_outer_angle + m_cone_size));
}

void Light::refresh_direction_from_transform()
{
	const glm::vec3 forward = transform.get_forward();
	if (glm::dot(forward, forward) > 1e-12f)
	{
		m_direction = glm::normalize(forward);
	}
}
