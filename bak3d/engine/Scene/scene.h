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

#include "Asset/model.h"
#include "Core/global_definitions.h"
#include "Objects/AdvancedParticleSystem.h"
#include "Objects/axis.h"
#include "Objects/camera.h"
#include "Objects/grid.h"
#include "Objects/light.h"
#include "Objects/Particle/particle_system.h"

/*
 * This is the class that contains all the scene's data. Runs in the main loop of the project.
 */
class Scene
{
public:
	Scene();
	~Scene();

	SceneObject* get_object_in_scene(SceneObjectType type, int index = 0);
	Camera* get_camera() const { return m_camera; }
	Grid* get_grid() const { return m_grid; }

	template<typename T, typename... Args>
	T* instantiate(SceneObjectType category, SceneObject* parent, Args&&... args)
	{
		static_assert(std::is_base_of_v<SceneObject, T>, "instantiate<T> requires a SceneObject-derived type");

		auto owned = std::make_unique<T>(std::forward<Args>(args)...);
		T* raw = owned.get();

		SceneObject* attach_point = parent ? parent : m_root.get();
		attach_point->add_child(std::move(owned));

		m_category_index[category].push_back(raw);
		return raw;
	}

	RenderableObject* instantiate_model(const ModelRef& model, SceneObject* parent = nullptr);

	void destroy(SceneObject* obj);

	void update(float dt) const;

	const std::vector<SceneObject*>& get_all(const SceneObjectType category) const
	{
		static const std::vector<SceneObject*> empty;
		const auto it = m_category_index.find(category);
		return it != m_category_index.end() ? it->second : empty;
	}

	SceneObject* get_root() const { return m_root.get(); }
private:
	std::unique_ptr<SceneObject> m_root;
	std::unordered_map<SceneObjectType, std::vector<SceneObject*>> m_category_index;

	Camera* m_camera;
	Grid* m_grid;
};