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

#include <glm/ext.hpp>
#include <filesystem>

#include "scene.h"

#include "editor.h"
#include "Asset/resource_manager.h"
#include "Core/global_settings.h"
#include "Objects/camera.h"

using namespace std;

Scene* Scene::instance;

Scene::Scene()
{
	instance = this;

	// Camera Setup
	m_camera = new Camera(glm::vec3(10.0f, 5.0f, 10.0f), // position
						  glm::vec3(0.0f, 0.0f, 0.0f),   // lookat
						  glm::vec3(0.0f, 1.0f, 0.0f),   // up
						  10.0f,  // speed
						  315.0f, // horizontal angle
						  30.0f,  // vertical angle
						  45.0f); // zoom
	
	// Grid Setup
	m_grid = new Grid(ResourceManager::get_material("grid"));
	m_scene_objects[SceneObjectType::Grid] = m_grid;
	
	m_axis = new Axis(ResourceManager::get_material("line"));
	m_scene_objects[SceneObjectType::Axis] = m_axis;

	// Light Setup
	auto initial_light_scaling_value = GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::Light_Scaling);
	m_light = new Light(glm::vec3(-5.0f, 5.0f, 5.0f),
		glm::vec3(initial_light_scaling_value, initial_light_scaling_value, initial_light_scaling_value),
		ResourceManager::get_material("light_icon"));
	m_scene_objects[SceneObjectType::Light] = m_light;

	// Model setup (empty for now)
	m_model = nullptr;
	m_particle_system = nullptr;
	m_advanced_particle_system = nullptr;
}

Scene::~Scene()
{
	delete m_grid;
	delete m_camera;
	delete m_particle_system;
	delete m_advanced_particle_system;
	delete m_light;
	delete m_axis;
	m_scene_objects.clear();
}

void Scene::update(float dt) const
{
	m_camera->update(dt);
	if (GlobalSettings::get_global_setting_value<bool>(GlobalSettingOption::Light_Enabled))
	{
		m_light->update(dt);
	}
	if (m_particle_system)
	{
		m_particle_system->update(dt);
	}
	if (m_advanced_particle_system)
	{
		m_advanced_particle_system->update(dt);
	}
}

ParticleSystem* Scene::spawn_particle_system()
{
	m_particle_system = new ParticleSystem();
	m_scene_objects[SceneObjectType::ParticleSystem] = m_particle_system;
	return m_particle_system;
}

void Scene::despawn_particle_system()
{
	delete m_particle_system;
	m_particle_system = nullptr;
}

AdvancedParticleSystem* Scene::spawn_advanced_particle_system()
{
	m_advanced_particle_system = new AdvancedParticleSystem();
	m_scene_objects[SceneObjectType::AdvancedParticleSystem] = m_advanced_particle_system;
	return m_advanced_particle_system;
}

void Scene::despawn_advanced_particle_system()
{
	delete m_advanced_particle_system;
	m_advanced_particle_system = nullptr;
}
