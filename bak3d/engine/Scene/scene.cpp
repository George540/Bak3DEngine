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

#include <filesystem>
#include <glm/ext.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "scene.h"

#include <ranges>

#include "editor.h"
#include "Asset/model.h"
#include "Asset/resource_manager.h"
#include "Core/global_settings.h"
#include "Input/event_manager.h"
#include "Objects/camera.h"
#include "Objects/grid.h"
#include "Objects/light.h"

using namespace std;

Scene::Scene()
{
	initialize_default_scene_objects();
}

Scene::~Scene()
{

}

void Scene::instantiate_model(const ModelRef& model, SceneObject* parent, glm::vec3 position)
{
    if (!model || !model->get_root_node())
    {
        return;
    }

    const ModelNode* root_node = model->get_root_node();

    ModelNodeObject* model_root = instantiate<ModelNodeObject>(parent, position, model->get_object_name());

    for (const MeshRef& mesh_ref : root_node->meshes)
    {
        const Mesh* mesh_object = instantiate<Mesh>(model_root, mesh_ref, mesh_ref->get_object_name());
        mesh_object->set_material(model->get_current_material());
    }
    
    for (auto& child_node : root_node->children)
    {
        instantiate_model_mesh(model, model_root, child_node.get(), root_node->local_transform);
    }
}

void Scene::instantiate_model_mesh(const ModelRef& model, SceneObject* model_root, const ModelNode* model_node, const glm::mat4& accumulated_transform)
{
    const glm::mat4 node_transform = accumulated_transform * model_node->local_transform;

    if (!model_node->meshes.empty())
    {
        glm::vec3 scale, translation, skew;
        glm::vec4 perspective;
        glm::quat rotation;
        glm::decompose(node_transform, scale, rotation, translation, skew, perspective);

        for (const MeshRef& mesh_ref : model_node->meshes)
        {
            Mesh* mesh_object = instantiate<Mesh>(model_root, mesh_ref, mesh_ref->get_object_name());
            mesh_object->set_material(model->get_current_material());

            mesh_object->transform.set_local_position(translation);
            mesh_object->transform.set_local_euler_rotation(glm::degrees(glm::eulerAngles(rotation)));
            mesh_object->transform.set_local_scale(scale);
        }
    }

    for (auto& child : model_node->children)
    {
        instantiate_model_mesh(model, model_root, child.get(), node_transform);
    }
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

glm::vec3 Scene::process_spawn_position() const
{
    glm::vec3 spawn_position = glm::vec3(0.0f);
    if (m_current_camera)
    {
        spawn_position = m_current_camera->get_camera_position() + m_current_camera->get_forward_vector() * 10.0f;
    }
    return spawn_position;
}

void Scene::update(float dt)
{
	for (const auto& type_storage : m_scene_objects_indexed | views::values)
	{
		for (const auto& object : type_storage)
		{
			object->update(dt);
		}
	}

    delete_selected_object();
}

void Scene::initialize_default_scene_objects()
{
    m_root = make_unique<SceneObject>(glm::vec3(0.0f), "SceneRoot");

    m_current_camera = instantiate<Camera>(nullptr, glm::vec3(5.0f, 3.0f, 5.0f));

    instantiate<Grid>(nullptr);
    instantiate<Light>(nullptr, LightType::Point, glm::vec3(-2.5f, 2.5f, 2.5f));
    instantiate<Mesh>(nullptr, glm::vec3(0.0f), "Cube", ResourceManager::get_material("default_material"), "Cube");
    
    B3D_LOG_INFO("Scene initialized.");
}

std::string Scene::get_unique_object_name(const std::string& name) const
{
     bool name_exists = false;

    for (const auto& objects : m_scene_objects_indexed | views::values)
    {
        for (const SceneObject* object : objects)
        {
            assert(object);
            if (object->get_object_name() == name)
            {
                name_exists = true;
                break;
            }
        }

        if (name_exists)
        {
            break;
        }
    }

    // Name is already unique
    if (!name_exists)
    {
        return name;
    }

    // The requested name already exists. Treat the entire requested name as the base:
    // - Light -> Light_1
    // - Light_5 -> Light_5_1
    const std::string prefix = name + "_";
    int highest_suffix = 0;
    for (const auto& objects : m_scene_objects_indexed | views::values)
    {
        for (const SceneObject* object : objects)
        {
            assert(object);
            const std::string& other_name = object->get_object_name();

            if (!other_name.starts_with(prefix))
            {
                continue;
            }

            const std::string suffix = other_name.substr(prefix.size());

            // Only care about a pure numeric suffix
            if (suffix.empty())
            {
                continue;
            }

            if (!ranges::all_of(suffix, [](const char c){ return std::isdigit(static_cast<unsigned char>(c));}))
            {
                continue;
            }

            const int suffix_number = std::stoi(suffix);
            highest_suffix = std::max(highest_suffix, suffix_number);
        }
    }

    return name + "_" + std::to_string(highest_suffix + 1);
}

void Scene::register_object(SceneObject* object)
{
    assert(object);

    const SceneObjectType type = object->object_type;
    const std::string original_name = object->get_object_name();

    const std::string unique_name = get_unique_object_name(original_name);
    if (unique_name != original_name)
    {
        B3D_LOG_INFO( "Scene: object name '%s' already exists, renamed to '%s'.", original_name.c_str(), unique_name.c_str());
        object->set_object_name(unique_name);
    }

    // Generic category index
    m_scene_objects_indexed[type].push_back(object);

    // Specialized typed indexes
    switch (type)
    {
        case SceneObjectType::Camera:
        {
            auto* camera = dynamic_cast<Camera*>(object);
            assert(camera);
            m_cameras.push_back(camera);
            break;
        }
        case SceneObjectType::Debug:
        {
            auto* renderable = dynamic_cast<RenderableObject*>(object);
            assert(renderable);
            m_debug_geometry.push_back(renderable);
            break;
        }
        case SceneObjectType::Light:
        {
            auto* light = dynamic_cast<Light*>(object);
            assert(light);
            m_lights.push_back(light);
            break;
        }
        case SceneObjectType::Mesh:
        {
            auto* mesh = dynamic_cast<Mesh*>(object);
            assert(mesh);
            m_meshes.push_back(mesh);
            break;
        }
        case SceneObjectType::Model:
            break;
        case SceneObjectType::ParticleSystem:
        {
            auto* particle_system = dynamic_cast<ParticleSystem*>(object);
            assert(particle_system);
            m_particle_systems.push_back(particle_system);
            break;
        }
        case SceneObjectType::AdvancedParticleSystem:
        {
            auto* particle_system = dynamic_cast<AdvancedParticleSystem*>(object);
            assert(particle_system);
            m_advanced_particle_systems.push_back(particle_system);
            break;
        }
        case SceneObjectType::Max:
		default:
            assert(false && "Cannot register SceneObjectType::Max");
            break;
    }
}

void Scene::unregister_object(SceneObject* object)
{
    assert(object);

    const SceneObjectType type = object->object_type;

    if (const auto indexed_it = m_scene_objects_indexed.find(type); indexed_it != m_scene_objects_indexed.end())
    {
        auto& storage = indexed_it->second;

        erase(storage, object);

        // Optional: remove empty category vectors.
        if (storage.empty())
        {
            m_scene_objects_indexed.erase(indexed_it);
        }
    }

    switch (type)
    {
        case SceneObjectType::Camera:
        {
            auto* camera = dynamic_cast<Camera*>(object);
            assert(camera);
            erase(m_cameras, camera);
            break;
        }
        case SceneObjectType::Debug:
        {
            auto* renderable = dynamic_cast<RenderableObject*>(object);
            assert(renderable);
            erase(m_debug_geometry, renderable);
            break;
        }
        case SceneObjectType::Light:
        {
            auto* light = dynamic_cast<Light*>(object);
            assert(light);
            erase(m_lights, light);
            break;
        }
        case SceneObjectType::Mesh:
        {
            auto* mesh = dynamic_cast<Mesh*>(object);
            assert(mesh);
            erase(m_meshes, mesh);
            break;
        }
        case SceneObjectType::Model:
            break;
        case SceneObjectType::ParticleSystem:
        {
            auto* particle_system = dynamic_cast<ParticleSystem*>(object);
            assert(particle_system);
            erase(m_particle_systems, particle_system);
            break;
        }
        case SceneObjectType::AdvancedParticleSystem:
        {
            auto* particle_system = dynamic_cast<AdvancedParticleSystem*>(object);
            assert(particle_system);
            erase(m_advanced_particle_systems, particle_system);
            break;
        }
        case SceneObjectType::Max:
		default:
            assert(false && "Cannot unregister SceneObjectType::Max");
            break;
    }
}

void Scene::delete_selected_object()
{
    if (EventManager::is_delete_key_down() && m_selected_scene_object)
    {
        destroy(m_selected_scene_object);
        m_selected_scene_object = nullptr;
    }
}

vector<SceneObject*> Scene::get_all_objects_of_type(const SceneObjectType type)
{
	return m_scene_objects_indexed[type];
}
