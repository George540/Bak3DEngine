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

#include "material.h"

#include <fstream>
#include <json.hpp>

#include "file_loader.h"
#include "resource_manager.h"
#include "shader.h"

using namespace std;
using ordered_json = nlohmann::ordered_json;

namespace
{
    glm::vec2 json_to_vec2(const nlohmann::json& v)
    {
        return { v[0].get<float>(), v[1].get<float>() };
    }

    glm::vec3 json_to_vec3(const nlohmann::json& v)
    {
        return { v[0].get<float>(), v[1].get<float>(), v[2].get<float>() };
    }

    glm::vec4 json_to_vec4(const nlohmann::json& v)
    {
        return { v[0].get<float>(), v[1].get<float>(), v[2].get<float>(), v[3].get<float>() };
    }

    glm::mat2 json_to_mat2(const nlohmann::json& v)
    {
        return glm::mat2{
            v[0].get<float>(), v[1].get<float>(),
            v[2].get<float>(), v[3].get<float>()
        };
    }

    glm::mat3 json_to_mat3(const nlohmann::json& v)
    {
        return glm::mat3{
            v[0].get<float>(), v[1].get<float>(), v[2].get<float>(),
            v[3].get<float>(), v[4].get<float>(), v[5].get<float>(),
            v[6].get<float>(), v[7].get<float>(), v[8].get<float>()
        };
    }

    glm::mat4 json_to_mat4(const nlohmann::json& v)
    {
        return glm::mat4{
            v[0].get<float>(),  v[1].get<float>(),  v[2].get<float>(),  v[3].get<float>(),
            v[4].get<float>(),  v[5].get<float>(),  v[6].get<float>(),  v[7].get<float>(),
            v[8].get<float>(),  v[9].get<float>(),  v[10].get<float>(), v[11].get<float>(),
            v[12].get<float>(), v[13].get<float>(), v[14].get<float>(), v[15].get<float>()
        };
    }
}

Material::Material(const string& path, const string& file_name, const ShaderRef& shader)
    : Asset(path, file_name)
{
    if (shader)
    {
        m_shader = shader;
    }
    else
    {
        load_from_file(path);
    }
}

Material::Material(const Material& other_material)
    : Asset(other_material.get_path(), other_material.get_file_name() + "_copy")
{
    m_shader = other_material.m_shader;
    
    m_uniform_flags = other_material.m_uniform_flags;
    m_int_uniforms = other_material.m_int_uniforms;
    m_uint_uniforms = other_material.m_uint_uniforms;
    m_float_uniforms = other_material.m_float_uniforms;
    m_vec2_uniforms = other_material.m_vec2_uniforms;
    m_vec3_uniforms = other_material.m_vec3_uniforms;
    m_vec4_uniforms = other_material.m_vec4_uniforms;
    m_mat2_uniforms = other_material.m_mat2_uniforms;
    m_mat3_uniforms = other_material.m_mat3_uniforms;
    m_mat4_uniforms = other_material.m_mat4_uniforms;
}

void Material::load_from_file(const string& path)
{
    ifstream file(path);
    if (!file.is_open())
    {
        throw runtime_error("Cannot open material: " + path);
    }

    ordered_json j;
    file >> j;

    const string material_name = FileLoader::get_name_from_path(path);
    const string shader_name   = j.at("shader").get<string>();

    m_shader = ResourceManager::get_shader(shader_name);

    for (auto& [uniform_name, entry] : j.at("uniforms").items())
    {
        const string type = entry.at("type").get<string>();
        const auto& val = entry.at("value");

        if      (type == "bool")  set_bool (uniform_name, val.get<bool>());
        else if (type == "int")   set_int  (uniform_name, val.get<int>());
        else if (type == "uint")  set_uint (uniform_name, val.get<unsigned int>());
        else if (type == "float") set_float(uniform_name, val.get<float>());
        else if (type == "vec2")  set_vec2 (uniform_name, json_to_vec2(val));
        else if (type == "vec3")  set_vec3 (uniform_name, json_to_vec3(val));
        else if (type == "vec4")  set_vec4 (uniform_name, json_to_vec4(val));
        else if (type == "mat2")  set_mat2 (uniform_name, json_to_mat2(val));
        else if (type == "mat3")  set_mat3 (uniform_name, json_to_mat3(val));
        else if (type == "mat4")  set_mat4 (uniform_name, json_to_mat4(val));
        else if (type == "sampler2D")
        {
            const string texture_name = val.get<string>();
            if (TextureRef texture = ResourceManager::get_texture(texture_name))
            {
                if (uniform_name == "material.diffuse_texture")
                {
                    set_int(uniform_name, 0);
                    m_texture_names[aiTextureType_DIFFUSE] = texture_name;
                }
                else if (uniform_name == "material.specular_texture")
                {
                    set_int(uniform_name, 1);
                    m_texture_names[aiTextureType_SPECULAR] = texture_name;
                }
                else if (uniform_name == "material.normal_texture")
                {
                    set_int(uniform_name, 2);
                    m_texture_names[aiTextureType_NORMALS] = texture_name;
                }
            }
        }
        else
        {
            throw runtime_error("Unknown uniform type: " + type);
        }
    }

    apply();
}

void Material::save_to_file() const
{
    if (m_path.empty())
    {
        B3D_LOG_ERROR("Material has no path to save to.");
        return;
    }

    // Create directories if they don't exist
    std::filesystem::create_directories(std::filesystem::path(m_path).parent_path());

    ordered_json j;

    j["shader"] = m_shader->get_object_name();

    ordered_json uniforms;

    for (const auto& [name, value] : m_uniform_flags)
        uniforms[name] = { {"type", "bool"}, {"value", value} };

    for (const auto& [name, value] : m_int_uniforms)
    {
        if (name == "material.diffuse_texture" || name == "material.specular_texture" || name == "material.normal_texture")
        {
            continue;
        }
        uniforms[name] = { {"type", "int"}, {"value", value} };
    }

    for (const auto& [name, value] : m_uint_uniforms)
        uniforms[name] = { {"type", "uint"}, {"value", value} };

    for (const auto& [name, value] : m_float_uniforms)
        uniforms[name] = { {"type", "float"}, {"value", value} };

    for (const auto& [name, value] : m_vec2_uniforms)
        uniforms[name] = { {"type", "vec2"}, {"value", {value.x, value.y}} };

    for (const auto& [name, value] : m_vec3_uniforms)
        uniforms[name] = { {"type", "vec3"}, {"value", {value.x, value.y, value.z}} };

    for (const auto& [name, value] : m_vec4_uniforms)
        uniforms[name] = { {"type", "vec4"}, {"value", {value.x, value.y, value.z, value.w}} };

    for (const auto& [name, value] : m_mat2_uniforms)
        uniforms[name] = { {"type", "mat2"}, {"value", {
            value[0][0], value[0][1],
            value[1][0], value[1][1]
        }}};

    for (const auto& [name, value] : m_mat3_uniforms)
        uniforms[name] = { {"type", "mat3"}, {"value", {
            value[0][0], value[0][1], value[0][2],
            value[1][0], value[1][1], value[1][2],
            value[2][0], value[2][1], value[2][2]
        }}};

    for (const auto& [name, value] : m_mat4_uniforms)
        uniforms[name] = { {"type", "mat4"}, {"value", {
            value[0][0], value[0][1], value[0][2], value[0][3],
            value[1][0], value[1][1], value[1][2], value[1][3],
            value[2][0], value[2][1], value[2][2], value[2][3],
            value[3][0], value[3][1], value[3][2], value[3][3]
        }}};

    if (m_texture_names.contains(aiTextureType_DIFFUSE))
    {
        uniforms["material.diffuse_texture"] = {
            {"type", "sampler2D"},
            {"value", m_texture_names.at(aiTextureType_DIFFUSE)}
        };
    }

    if (m_texture_names.contains(aiTextureType_SPECULAR))
    {
        uniforms["material.specular_texture"] = {
            {"type", "sampler2D"},
            {"value", m_texture_names.at(aiTextureType_SPECULAR)}
        };
    }

    if (m_texture_names.contains(aiTextureType_NORMALS))
    {
        uniforms["material.normal_texture"] = {
            {"type", "sampler2D"},
            {"value", m_texture_names.at(aiTextureType_NORMALS)}
        };
    }

    j["uniforms"] = uniforms;

    // Write to file
    std::ofstream file(m_path);
    if (!file.is_open())
    {
        B3D_LOG_ERROR("Could not open material file for writing: %s", m_path.c_str());
        return;
    }

    file << j.dump(4); // 4 = indent spaces
    B3D_LOG_INFO("Material saved to: %s", m_path.c_str());
}

void Material::apply()
{
    if (!m_shader) return;

    m_shader->use();

    for (const auto& [name, value] : m_uniform_flags)
    {
        m_shader->set_bool(name, value);
    }

    for (const auto& [name, value] : m_uint_uniforms)
    {
        m_shader->set_uint(name, value);
    }

    for (const auto& [name, value] : m_int_uniforms)
    {
        m_shader->set_int(name, value);
    }

    for (const auto& [name, value] : m_float_uniforms)
    {
        m_shader->set_float(name, value);
    }

    for (const auto& [name, value] : m_vec2_uniforms)
    {
        m_shader->set_vec2(name, value);
    }

    for (const auto& [name, value] : m_vec3_uniforms)
    {
        m_shader->set_vec3(name, value);
    }

    for (const auto& [name, value] : m_vec4_uniforms)
    {
        m_shader->set_vec4(name, value);
    }

    for (const auto& [name, value] : m_mat2_uniforms)
    {
        m_shader->set_mat2(name, value);
    }

    for (const auto& [name, value] : m_mat3_uniforms)
    {
        m_shader->set_mat3(name, value);
    }

    for (const auto& [name, value] : m_mat4_uniforms)
    {
        m_shader->set_mat4(name, value);
    }
}

void Material::bind_textures_cache()
{
    if (m_texture_names.empty())
    {
        return;
    }

    Texture2D::unbind_all(m_texture_names.size());

    for (const auto& [type, value] : m_texture_names)
    {
        const TextureRef texture = ResourceManager::get_texture(value);
        if (get_bool("material.use_diffuse_texture") && type == aiTextureType_DIFFUSE)
        {
            texture->bind(0);
            set_int("material.diffuse_texture", 0);
        }
        if (get_bool("material.use_specular_texture") && type == aiTextureType_SPECULAR)
        {
            texture->bind(1);
            set_int("material.specular_texture", 1);
        }
        if (get_bool("material.use_normal_texture") && type == aiTextureType_NORMALS)
        {
            texture->bind(2);
            set_int("material.normal_texture", 2);
        }
    }
}

std::string Material::get_texture_by_type(const aiTextureType& texture_type)
{
    return m_texture_names[texture_type];
}

void Material::set_texture_by_type(const aiTextureType& texture_type, const std::string& texture_name)
{
    if (texture_name.empty())
        return;
    m_texture_names[texture_type] = texture_name;

    // Wire up the sampler slot index so the shader knows which unit to read.
    if (texture_type == aiTextureType_DIFFUSE)
        set_int("material.diffuse_texture", 0);
    else if (texture_type == aiTextureType_SPECULAR)
        set_int("material.specular_texture", 1);
    else if (texture_type == aiTextureType_NORMALS)
        set_int("material.normal_texture", 2);
}

void Material::remove_texture_by_type(const aiTextureType& key_name)
{
    if (!m_texture_names.contains(key_name))
        return;

    m_texture_names.erase(key_name);

    // Undo the sampler binding and disable the toggle so the shader falls back to the scalar surface parameter for this channel.
    if (key_name == aiTextureType_DIFFUSE)
    {
        m_uniform_flags["material.use_diffuse_texture"]  = false;
        m_int_uniforms.erase("material.diffuse_texture");
    }
    else if (key_name == aiTextureType_SPECULAR)
    {
        m_uniform_flags["material.use_specular_texture"] = false;
        m_int_uniforms.erase("material.specular_texture");
    }
    else if (key_name == aiTextureType_NORMALS)
    {
        m_uniform_flags["material.use_normal_texture"]   = false;
        m_int_uniforms.erase("material.normal_texture");
    }
}

bool Material::has_uniform(const std::string& key_name) const
{
    return m_uniform_flags.contains(key_name)
            || m_int_uniforms.contains(key_name)
            || m_float_uniforms.contains(key_name)
            || m_vec2_uniforms.contains(key_name)
            || m_vec3_uniforms.contains(key_name)
            || m_vec4_uniforms.contains(key_name)
            || m_mat2_uniforms.contains(key_name)
            || m_mat3_uniforms.contains(key_name)
            || m_mat4_uniforms.contains(key_name);
}
