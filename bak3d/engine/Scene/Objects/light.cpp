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
#include "Input/event_manager.h"

#include <GLFW/glfw3.h>

#include "Asset/resource_manager.h"
#include "Asset/texture.h"
#include "Core/global_settings.h"
#include "Core/logger.h"
#include "Scene/scene.h"

Light::Light(glm::vec3 position, glm::vec3 scaling, const MaterialRef& material) :
	RenderableObject(material, position, "Light")
{
	transform.set_local_scale(scaling);
	transform.compute_model_matrix();

	// @TODO: Replace with struct payload instead of manual size
	m_light_data_ubo = std::make_unique<UniformBuffer>(6 * VEC4_SIZE /*Temporary size*/, nullptr, 1, GL_DYNAMIC_DRAW);

	m_diffuse = glm::vec3(1.0f);
	m_ambient = glm::vec3(0.3f);
	m_specular = glm::vec3(0.3f);

	m_attenuation_radius = 32.0f;

	m_cone_size = 1.0f;
	m_inner_cut_off = glm::cos(glm::radians(12.5f));
	m_outer_cut_off = glm::cos(glm::radians(17.5f));

	m_horizontal_angle = transform.get_global_position().x;
	m_vertical_angle = transform.get_global_position().y;
	m_distance_offset = glm::distance(transform.get_global_position(), glm::vec3(0.0f));

	m_mesh_slot = make_mesh_slot(ResourceManager::get_mesh("Quad"));
	set_texture_by_type(LightType::Point);

	B3D_LOG_INFO("Light created.");
}

void Light::update(float dt)
{
	m_horizontal_angle = GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::Light_HorizontalRotation);
	m_vertical_angle = GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::Light_VerticalRotation);
	m_distance_offset = GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::Light_OriginDistance);
	m_intensity = GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::Light_Intensity);
	const float sprite_scaling = GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::Light_Scaling);

	const auto theta = glm::radians(m_horizontal_angle);
	const auto phi = glm::radians(m_vertical_angle);
	glm::vec3 position = glm::vec3(cosf(phi) * cosf(theta), sinf(phi), -cosf(phi) * sinf(theta));

	position *= m_distance_offset;
	glm::vec3 scale = glm::vec3(sprite_scaling, sprite_scaling, sprite_scaling);

	transform.set_local_position(position);
	transform.set_local_scale(scale);
	transform.compute_model_matrix();
	m_direction = glm::normalize(glm::vec3(0.0f) - position);

	const glm::vec4 diffuse = GlobalSettings::get_global_setting_value<glm::vec4>(GlobalSettingOption::Light_Color);
	m_diffuse = glm::vec3(diffuse.r, diffuse.g, diffuse.b);

	const LightType type = static_cast<LightType>(GlobalSettings::get_global_setting_value<uint32_t>(GlobalSettingOption::Light_Type));
	set_texture_by_type(type);

	m_cone_size = GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::Light_Spot_ConeAngle_Size);
	m_attenuation_radius = GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::Light_Point_Attenuation_Radius);
	m_inner_cut_off = glm::cos(glm::radians(GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::Light_Spot_ConeAngle_Inner_CutOff) + m_cone_size));
	m_outer_cut_off = glm::cos(glm::radians(GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::Light_Spot_ConeAngle_Outer_CutOff) + m_cone_size));

	update_light_data_ubo();

	RenderableObject::update(dt);
}

void Light::draw() const
{
	(*m_material_slot)->set_vec4("diffuseColor", glm::vec4(m_diffuse, 1.0f));

	m_sprite_texture->bind(0);
	
	RenderableObject::draw();

	Texture2D::unbind();
}

void Light::set_attenuation(const float radius)
{
	m_attenuation_radius = radius;
}

void Light::set_cone_angles(const float inner_degrees, const float outer_degrees)
{
	m_inner_cut_off = glm::cos(glm::radians(inner_degrees + m_cone_size));
	m_outer_cut_off = glm::cos(glm::radians(outer_degrees + m_cone_size));
}

void Light::update_light_data_ubo() const
{
	m_light_data_ubo->bind();

	const glm::vec3 position = transform.get_global_position();
	// vec4 position
	m_light_data_ubo->bind_buffer_sub_data(&position,        VEC3_SIZE,  0 * VEC4_SIZE + 0);
	m_light_data_ubo->bind_buffer_sub_data(&m_inner_cut_off, FLOAT_SIZE, 0 * VEC4_SIZE + VEC3_SIZE);

	// vec4 direction
	m_light_data_ubo->bind_buffer_sub_data(&m_direction,			 VEC3_SIZE,  1 * VEC4_SIZE + 0);
	m_light_data_ubo->bind_buffer_sub_data(&m_outer_cut_off,		 FLOAT_SIZE, 1 * VEC4_SIZE + VEC3_SIZE);

	// vec4 ambient (.a = radius)
	m_light_data_ubo->bind_buffer_sub_data(&m_ambient,			 VEC3_SIZE,  2 * VEC4_SIZE + 0);
	m_light_data_ubo->bind_buffer_sub_data(&m_attenuation_radius, FLOAT_SIZE, 2 * VEC4_SIZE + VEC3_SIZE);

	// vec4 diffuse (.a = intensity)
	m_light_data_ubo->bind_buffer_sub_data(&m_diffuse,			 VEC3_SIZE,  3 * VEC4_SIZE + 0);
	m_light_data_ubo->bind_buffer_sub_data(&m_intensity,			 FLOAT_SIZE, 3 * VEC4_SIZE + VEC3_SIZE);

	// vec4 specular (.a = unused)
	m_light_data_ubo->bind_buffer_sub_data(&m_specular,			 VEC3_SIZE,  4 * VEC4_SIZE + 0);

	// int type
	const int32_t type = static_cast<int32_t>(m_type);
	m_light_data_ubo->bind_buffer_sub_data(&type,				 INT_SIZE,   5 * VEC4_SIZE);

	m_light_data_ubo->unbind();
}

void Light::set_texture_by_type(const LightType type)
{
	if (m_type == type)
	{
		return;
	}

	m_type = type;
	switch (m_type)
	{
		case LightType::Directional: m_sprite_texture = ResourceManager::get_texture("directional_light_icon.png"); break;
		case LightType::Point: m_sprite_texture = ResourceManager::get_texture("point_light_icon.png"); break;
		case LightType::Spot: m_sprite_texture = ResourceManager::get_texture("spot_light_icon.png"); break;
		case LightType::Area: m_sprite_texture = ResourceManager::get_texture("area_light_icon.png"); break;
		default: m_sprite_texture = ResourceManager::get_texture("point_light_icon.png"); break;
	}
}