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
 * This is the class that contains all the scene's data, such as references to cameras, lights, models, grids, etc.
 * Runs in the main loop of the project and is only one singleton instance (at the moment).
 */
class Scene
{
public:
	Scene();
	~Scene();

	static Scene* get_instance() { return instance; }

	void update(float dt) const;

	Camera* get_camera() const { return m_camera; }
	Light* get_active_light() const { return m_light; }

	RenderableObject* get_object_in_scene(SceneObjectType object_type) const { return m_scene_objects.at(object_type); }

	Model* get_model() const { return m_model; }
	void set_model(Model* model) { m_model = model; }

	ParticleSystem* get_particle_system() const { return m_particle_system; }
	void set_particle_system(ParticleSystem* model) { m_particle_system = model; }

	AdvancedParticleSystem* get_advanced_particle_system() const { return m_advanced_particle_system; }
	void set_advanced_particle_system(AdvancedParticleSystem* model) { m_advanced_particle_system = model; }

	ParticleSystem* spawn_particle_system();
	void despawn_particle_system();

	AdvancedParticleSystem* spawn_advanced_particle_system();
	void despawn_advanced_particle_system();

	static Scene* instance;
private:
	std::unordered_map<SceneObjectType, RenderableObject*> m_scene_objects;
	ParticleSystem* m_particle_system;
	AdvancedParticleSystem* m_advanced_particle_system;
	Camera* m_camera;
	Model* m_model; // Asset, not an Object
	Light* m_light;
	Grid* m_grid;
	Axis* m_axis;
};