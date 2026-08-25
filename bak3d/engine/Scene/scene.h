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

#include <map>

#include "Objects/AdvancedParticleSystem.h"
#include "Objects/camera.h"
#include "Objects/light.h"
#include "Objects/mesh.h"
#include "Objects/renderable_object.h"
#include "Objects/Particle/particle_system.h"

/*
 * This is the class that contains all the scene's data. Runs in the main loop of the project.
 */
class Scene
{
public:
	Scene();
	~Scene();

	template<typename T, typename... Args>
	T* instantiate(const SceneObjectType category, SceneObject* parent, Args&&... args)
	{
		static_assert(std::is_base_of_v<SceneObject, T>, "instantiate<T> requires a SceneObject-derived type");

		auto owned = std::make_unique<T>(std::forward<Args>(args)...);
		T* raw = owned.get();

		SceneObject* attach_point = parent ? parent : m_root.get();
		attach_point->add_child(std::move(owned));

		m_scene_objects_indexed[category].push_back(raw);
		if constexpr (std::is_same_v<T, Camera>)
			m_cameras.push_back(raw);
		else if constexpr (std::is_same_v<T, Light>)
			m_lights.push_back(raw);
		else if constexpr (std::is_same_v<T, Mesh>)
			m_meshes.push_back(raw);
		else if constexpr (std::is_same_v<T, ParticleSystem>)
			m_particle_systems.push_back(raw);
		else if constexpr (std::is_same_v<T, AdvancedParticleSystem>)
			m_advanced_particle_systems.push_back(raw);
		else if constexpr (std::is_base_of_v<RenderableObject, T>) // Leftover geometry, most likely debug
			m_debug_geometry.push_back(raw);

		return raw;
	}

	SceneObject* get_root() const { return m_root.get(); }
	Camera* get_current_camera() const { return m_current_camera; }
	std::vector<SceneObject*> get_all_objects_of_type(SceneObjectType type);
	SceneObject* get_object_in_scene(SceneObjectType type, int index = 0);

	std::vector<Camera*>& get_all_cameras() { return m_cameras; }
	std::vector<Light*>& get_all_lights() { return m_lights; }
	std::vector<Mesh*>& get_all_meshes() { return m_meshes; }
	std::vector<RenderableObject*> get_all_debug_geometry() { return m_debug_geometry; }
	std::vector<ParticleSystem*>& get_all_particle_systems() { return m_particle_systems; }
	std::vector<AdvancedParticleSystem*>& get_all_advanced_particle_systems() { return m_advanced_particle_systems; }

	void update(float dt) const;

private:
	Camera* m_current_camera = nullptr;
	
	std::unique_ptr<SceneObject> m_root;
	std::map<SceneObjectType, std::vector<SceneObject*>> m_scene_objects_indexed; // <object name, scene object ptr>

	std::vector<Camera*> m_cameras;
	std::vector<RenderableObject*> m_debug_geometry;
	std::vector<Light*> m_lights;
	std::vector<Mesh*> m_meshes;
	std::vector<ParticleSystem*> m_particle_systems;
	std::vector<AdvancedParticleSystem*> m_advanced_particle_systems;
};
