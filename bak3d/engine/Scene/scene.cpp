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
#include "Input/event_manager.h"
#include "Objects/camera.h"
#include "Objects/grid.h"
#include "Objects/light.h"
#include "Renderer/renderer.h"

using namespace std;

namespace
{
    bool is_previewable_shader(const ShaderRef& shader)
    {
        if (!shader.is_valid() || !shader->is_shader_compiled()) return false;
        const string& name = shader->get_object_name();
        return name == "gbuffer" || name == "lit";
    }

    // Draws deferred (gbuffer) materials through the forward "lit" shader, keeping textures and uniforms
    struct PreviewShaderOverride
    {
        vector<pair<Material*, ShaderRef>> saved;

        explicit PreviewShaderOverride(const vector<Mesh*>& meshes)
        {
            const ShaderRef lit = ResourceManager::get_shader("lit");
            if (!lit.is_valid() || !lit->is_shader_compiled())
            {
                return;
            }

            for (const Mesh* mesh : meshes)
            {
                if (!mesh->has_material())
                {
                    continue;
                }

                Material* material = mesh->get_material().operator->();
                
                if (ranges::any_of(saved, [material](const auto& e){ return e.first == material; }))
                {
                    continue;
                }

                const ShaderRef original = material->get_shader();
                if (original.is_valid() && original->get_object_name() == "gbuffer")
                {
                    saved.emplace_back(material, original);
                    material->set_shader(lit);
                }
            }
        }

        ~PreviewShaderOverride()
        {
            for (auto& [material, shader] : saved) material->set_shader(shader);
        }
    };
}

Scene::Scene(const bool is_preview_scene)
{
    m_root = make_unique<SceneObject>(glm::vec3(0.0f), "SceneRoot");

    m_current_camera = instantiate<Camera>(nullptr, glm::vec3(5.0f, 3.0f, 5.0f));

    if (is_preview_scene)
    {
        initialize_preview_scene_objects();
    }
    else
    {
        initialize_default_scene_objects();
    }
}

Scene::~Scene()
{

}

ModelNodeObject* Scene::instantiate_model(const ModelRef& model, SceneObject* parent, glm::vec3 position)
{
    if (!model || !model->get_root_node())
    {
        return nullptr;
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

    return model_root;
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
    instantiate<Grid>(nullptr);
    instantiate<Light>(nullptr, LightType::Directional, glm::vec3(-2.5f, 2.5f, 2.5f));
    instantiate<Mesh>(nullptr, glm::vec3(0.0f, 1.0f, 0.0f), "Cube", ResourceManager::get_material("default_material"), "Cube");
    const auto plane_object = instantiate<Mesh>(nullptr, glm::vec3(0.0f), "Plane", ResourceManager::get_material("default_material"), "Plane");
    plane_object->transform.set_local_scale(glm::vec3(10.0f, 1.0f, 10.0f));
    
    B3D_LOG_INFO("Default Scene initialized.");
}

void Scene::initialize_preview_scene_objects()
{
    Light* light = instantiate<Light>(nullptr, LightType::Directional, glm::vec3(-2.5f, 2.5f, 2.5f));
    light->set_direction(glm::normalize(-light->transform.get_local_position()));

    B3D_LOG_INFO("Preview scene initialized.");
}

string Scene::get_unique_object_name(const string& name) const
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
    const string prefix = name + "_";
    int highest_suffix = 0;
    for (const auto& objects : m_scene_objects_indexed | views::values)
    {
        for (const SceneObject* object : objects)
        {
            assert(object);
            const string& other_name = object->get_object_name();

            if (!other_name.starts_with(prefix))
            {
                continue;
            }

            const string suffix = other_name.substr(prefix.size());

            // Only care about a pure numeric suffix
            if (suffix.empty())
            {
                continue;
            }

            if (!ranges::all_of(suffix, [](const char c){ return isdigit(static_cast<unsigned char>(c));}))
            {
                continue;
            }

            const int suffix_number = stoi(suffix);
            highest_suffix = max(highest_suffix, suffix_number);
        }
    }

    return name + "_" + to_string(highest_suffix + 1);
}

void Scene::register_object(SceneObject* object)
{
    assert(object);

    const SceneObjectType type = object->object_type;
    const string original_name = object->get_object_name();

    const string unique_name = get_unique_object_name(original_name);
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

GLuint Scene::render_preview_thumbnail() const
{
    m_root->force_update_self_and_children(); // preview scene is never ticked
    frame_camera_on_meshes();
    m_current_camera->force_update_self_and_children();
    m_current_camera->update(0.0f); // upload UBO and rebinds binding 0

    const PreviewShaderOverride shader_override(m_meshes);
    return Renderer::process_asset_captures();
}

void Scene::frame_camera_on_meshes() const
{
    glm::vec3 min_p(FLT_MAX), max_p(-FLT_MAX);

    // Use simple AABB measurements from the min/max vertex world positions to frame the asset inside the preview image correctly.
    // Small or big asset, it will be framed inside the thumbnail correctly.
    for (const Mesh* mesh : m_meshes)
    {
        if (!mesh->has_mesh())
        {
            continue;
        }
        const auto* data = dynamic_cast<const MeshData*>(mesh->get_mesh().operator->());
        if (!data)
        {
            continue;
        }

        const glm::mat4 model = mesh->transform.get_global_model_matrix();
        for (const Vertex& v : data->get_vertices())
        {
            const glm::vec3 p = glm::vec3(model * glm::vec4(v.position, 1.0f));
            min_p = glm::min(min_p, p);
            max_p = glm::max(max_p, p);
        }
    }

    if (min_p.x > max_p.x) { min_p = glm::vec3(-0.5f); max_p = glm::vec3(0.5f); }

    const glm::vec3 center = (min_p + max_p) * 0.5f;
    const float radius = glm::max(glm::length(max_p - min_p) * 0.5f, 0.001f);
    const float distance = radius / glm::sin(glm::radians(22.5f)) * 1.15f; // camera fov is 45 deg

    const glm::vec3 offset_direction = glm::normalize(glm::vec3(0.6f, 0.5f, 1.0f)); // same side as the preview light
    const glm::vec3 look_direction = -offset_direction;

    const float pitch = glm::degrees(glm::asin(look_direction.y));
    const float yaw = glm::degrees(glm::atan(-look_direction.x, -look_direction.z));

    m_current_camera->transform.set_local_position(center + offset_direction * distance);
    m_current_camera->transform.set_local_euler_rotation(glm::vec3(pitch, yaw, 0.0f));
}

void Scene::finish_preview_capture(Asset* asset, SceneObject* preview_object)
{
    if (!preview_object)
    {
        return;
    }

    const GLuint thumbnail_id = render_preview_thumbnail();

    if (const GLuint old_id = asset->get_thumbnail_id(); old_id != 0)
    {
        glDeleteTextures(1, &old_id);
    }
    asset->set_thumbnail_id(thumbnail_id);

    destroy(preview_object);
}

void Scene::capture_all_asset_previews()
{
    // Temporarily set the viewport size to the thumbnail size to prepare the asset scene for capturing asset thumbnails
    const int original_w = EventManager::get_viewport_width();
    const int original_h = EventManager::get_viewport_height();
    EventManager::set_viewport_width(PREVIEW_THUMBNAIL_SIZE);
    EventManager::set_viewport_height(PREVIEW_THUMBNAIL_SIZE);

    for (const auto& model : ResourceManager::Models.all() | views::values) capture_model_preview(model);
    for (const auto& material : ResourceManager::Materials.all() | views::values) capture_material_preview(material);
    for (const auto& mesh : ResourceManager::Meshes.all() | views::values) capture_mesh_preview(mesh);

    EventManager::set_viewport_width(original_w);
    EventManager::set_viewport_height(original_h);
}

void Scene::capture_model_preview(const ModelRef& model)
{
    if (!model || !model->get_root_node())
    {
        return;
    }
    SceneObject* instance = instantiate_model(model, nullptr, glm::vec3(0.0f));
    finish_preview_capture(model.operator->(), instance);
}

void Scene::capture_material_preview(const MaterialRef& material)
{
    if (!material || !is_previewable_shader(material->get_shader()))
    {
        return;
    }
    SceneObject* instance = instantiate<Mesh>(nullptr, glm::vec3(0.0f), "MaterialPreview", material, "Sphere");
    finish_preview_capture(material.operator->(), instance);
}

void Scene::capture_mesh_preview(const MeshRef& mesh)
{
    // Only Mesh data can be previewed. Skip Grid/Quad layouts
    if (!mesh || !dynamic_cast<MeshData*>(mesh.operator->()))
    {
        return;
    }
    SceneObject* instance = instantiate<Mesh>(nullptr, mesh, mesh->get_object_name()); // default material
    finish_preview_capture(mesh.operator->(), instance);
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
