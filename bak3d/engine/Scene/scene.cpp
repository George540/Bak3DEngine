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

using namespace std;

Scene::Scene()
{
	m_root = std::make_unique<SceneObject>(glm::vec3(0.0f), "SceneRoot");

	// Camera Setup
	m_camera = new Camera(glm::vec3(10.0f, 5.0f, 10.0f), // position
						  glm::vec3(0.0f, 0.0f, 0.0f),   // lookat
						  glm::vec3(0.0f, 1.0f, 0.0f),   // up
						  10.0f,  // speed
						  315.0f, // horizontal angle
						  30.0f,  // vertical angle
						  45.0f); // zoom

	m_grid = new Grid();

	auto initial_light_scaling_value = GlobalSettings::get_global_setting_value<float>(GlobalSettingOption::Light_Scaling);

	Light* initial_light = instantiate<Light>(
		SceneObjectType::Light,
		nullptr,
		glm::vec3(-5.0f, 5.0f, 5.0f),
		glm::vec3(initial_light_scaling_value, initial_light_scaling_value, initial_light_scaling_value),
		ResourceManager::get_material("light_icon")
		);

	B3D_LOG_INFO("Scene initialized.");
}

Scene::~Scene()
{
	delete m_grid;
}

SceneObject* Scene::get_object_in_scene(const SceneObjectType type, const int index)
{
	SceneObject* ret = nullptr;
	if (m_category_index.contains(type) && m_category_index[type][0] )
	{
		ret = m_category_index[type][index];
	}

	return ret;
}

RenderableObject* Scene::instantiate_model(const ModelRef& model, SceneObject* parent)
{
	unique_ptr<RenderableObject> subtree(instantiate_model(model)); // from the previous step
	if (!subtree)
	{
		return nullptr;
	}

	RenderableObject* root = subtree.get();
	SceneObject* attach_point = parent ? parent : m_root.get();
	attach_point->add_child(std::move(subtree));

	// Register every node in the freshly-attached subtree, not just the root,
	// so SceneGraph/Details can find individual submeshes by category too.
	function<void(SceneObject*)> register_recursive = [&](SceneObject* node)
	{
		m_category_index[SceneObjectType::Model].push_back(node);
		for (auto& child : node->children)
		{
			register_recursive(child.get());
		}
	};
	register_recursive(root);

	return root;
}

void Scene::destroy(SceneObject* obj)
{
	if (!obj || !obj->parent)
	{
		return;
	}

	for (auto& list : m_category_index | views::values)
	{
		std::erase(list, obj);
	}

	auto& siblings = obj->parent->children;
	erase_if(siblings, [obj](const unique_ptr<SceneObject>& c) { return c.get() == obj; });
}

void Scene::update(float dt) const
{
	m_camera->update(dt);
}
