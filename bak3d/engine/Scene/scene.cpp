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

#include <ranges>

#include "editor.h"
#include "Asset/resource_manager.h"
#include "Core/global_settings.h"
#include "Objects/camera.h"
#include "Objects/grid.h"
#include "Objects/light.h"

using namespace std;

Scene::Scene()
{
	m_root = make_unique<SceneObject>(glm::vec3(0.0f), "SceneRoot");

	// Camera Setup
	m_current_camera = instantiate<Camera>(SceneObjectType::Camera,
	                                       nullptr,
	                                       glm::vec3(10.0f, 5.0f, 10.0f),
	                                       glm::vec3(0.0f, 0.0f, 0.0f),
	                                       glm::vec3(0.0f, 1.0f, 0.0f),
	                                       10.0f,
	                                       315.0f,
	                                       30.0f,
	                                       45.0f);

	instantiate<Grid>(SceneObjectType::Debug, nullptr);

	const auto initial_light_scaling_value = GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::Light_Scaling);
	instantiate<Light>(SceneObjectType::Light,
		nullptr,
		glm::vec3(-5.0f, 5.0f, 5.0f),
		glm::vec3(initial_light_scaling_value, initial_light_scaling_value, initial_light_scaling_value),
		ResourceManager::get_material("light_icon"));

	/*auto models = ResourceManager::Models;
	instantiate_model(ResourceManager::get_model("mushroom.obj"));#1#*/

	B3D_LOG_INFO("Scene initialized.");
}

Scene::~Scene()
{
	
}

SceneObject* Scene::get_object_in_scene(const SceneObjectType type, const int index)
{
	SceneObject* object = nullptr;
	if (m_scene_objects_indexed.contains(type) && m_scene_objects_indexed[type][0])
	{
		object = m_scene_objects_indexed[type][index];
	}
	return object;
}

void Scene::update(float dt) const
{
	for (const auto& type_storage : m_scene_objects_indexed | views::values)
	{
		for (const auto& object : type_storage)
		{
			object->update(dt);
		}
	}
}

std::vector<SceneObject*> Scene::get_all_objects_of_type(const SceneObjectType type)
{
	return m_scene_objects_indexed[type];
}
