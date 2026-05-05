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

#include "resource_manager.h"

#include <filesystem>
#include <ranges>

#include "file_loader.h"
#include "model.h"
#include "shader.h"
#include "texture.h"
#include "Core/logger.h"

using namespace std;

// Instantiate static variables
ResourceMap<Shader> ResourceManager::Shaders;
ResourceMap<Texture2D> ResourceManager::Textures;
ResourceMap<Material> ResourceManager::Materials;
ResourceMap<Model> ResourceManager::Models;

void ResourceManager::initialize()
{
    initialize_shaders();
    initialize_predefined_textures();
    initialize_predefined_materials();
    initialize_models();
}

void ResourceManager::reload_shaders()
{
    B3D_LOG_INFO("Reloading shaders...");
    initialize_shaders();
    B3D_LOG_INFO("Reloading shaders complete.");
}

void ResourceManager::initialize_shaders()
{
    // Get directory and files
    const auto directory = BAK3D_SHADERS_DIR;
    auto vertex_shader_files = FileLoader::get_files_by_type_with_path(directory, vert);
    auto fragment_shader_files = FileLoader::get_files_by_type_with_path(directory, frag);

    // Map shaders for lookup later
    unordered_map<string, string> vertex_shaders_map;
    for (const auto& [file_name, file_path] : vertex_shader_files)
    {
        string name = FileLoader::get_name_from_filename(file_name);
        vertex_shaders_map[name] = file_path;
    }
    unordered_map<string, string> fragment_shaders_map;
    for (const auto& [file_name, file_path] : fragment_shader_files)
    {
        string name = FileLoader::get_name_from_filename(file_name);
        fragment_shaders_map[name] = file_path;
    }

    unordered_map<string, string> shaders_to_match;
    vector<pair<string, string>> vertex_fragment_sources;

    // Iterate through vertex shader files
    for (const auto& [filename, filepath] : vertex_shader_files)
    {
        string vert_file_name = filename;
        string vert_path = filepath;

        string vert_name = FileLoader::get_name_from_filename(vert_file_name);

        // Combine vert and frag paths in pairs that have the same name
        if (fragment_shaders_map.contains(vert_name))
        {
            vertex_fragment_sources.emplace_back(vert_path, fragment_shaders_map[vert_name]); // <vertex path, fragment path>
        }
        else
        {
            // If they don't match, add vertex shader into a to-be-matched container
            shaders_to_match.emplace(vert_name, vert_path);
        }
    }

    // Find unmatched fragment shaders, if any, and add to the to-be-matched container
    for (const auto& [filename, filepath] : fragment_shader_files)
    {
        string frag_file_name = filename;
        string frag_path = filepath;

        string frag_name = FileLoader::get_name_from_filename(frag_file_name);

        if (!vertex_shaders_map.contains(frag_name))
        {
            shaders_to_match.emplace(frag_name, frag_path);
        }
    }

    auto test = shaders_to_match;

    // Create and bind shaders.
    for (const auto& shader_file_pair : vertex_fragment_sources)
    {
        // Use frag name for shader name, since it's possible that vertex shaders are common to others
        auto shader_path_string = string(shader_file_pair.second);
        auto shader_name = FileLoader::get_name_from_path(shader_path_string);

        Shader* new_shader = new Shader(
            shader_file_pair.first.c_str(),
            shader_file_pair.second.c_str(),
            shader_name);

        add_shader(shader_name, new_shader);
    }

    // If shaders that did not automatically match are found, manually match them with caution!
    // NOTE: This is used so far to match post process fragment shaders with the PostProcessingShader.vert vertex shader.
    //       See shader files for reference.
    if (!shaders_to_match.empty())
    {
        add_shader("ColorGradingShader", new Shader(
                shaders_to_match["PostProcessingShader"].c_str(),
                shaders_to_match["ColorGradingShader"].c_str(),
                "ColorGradingShader"));

        add_shader("KernelEffectShader", new Shader(
                shaders_to_match["PostProcessingShader"].c_str(),
                shaders_to_match["KernelEffectShader"].c_str(),
                "KernelEffectShader"));
    }

    auto shaders = Shaders;
    if (Shaders.empty())
    {
        B3D_LOG_ERROR("No shaders loaded! Program will crash");
    }
    else
    {
        B3D_LOG_INFO("Successfully compiled shaders. Number of shaders compiled: %d", Shaders.size());
    }
}

void ResourceManager::initialize_predefined_textures()
{
    auto image_files = FileLoader::get_files_by_type_with_path(string(BAK3D_ASSETS_DIR) + "/sprites", png);
    for (const auto& [file_name, file_path] : image_files)
    {
        add_texture(file_name, new Texture2D(file_path, file_name, aiTextureType_DIFFUSE));
    }

    image_files = FileLoader::get_files_by_type_with_path(string(BAK3D_ASSETS_DIR) + "/sprites", jpg);
    for (const auto& [file_name, file_path] : image_files)
    {
        add_texture(file_name, new Texture2D(file_path, file_name, aiTextureType_DIFFUSE));
    }

    B3D_LOG_INFO("Successfully loaded %d default sprite textures.", Textures.size());
}

void ResourceManager::initialize_models()
{
    const auto model_files = FileLoader::get_files_by_type_with_path(string(BAK3D_ASSETS_DIR) + "/models", obj);
    int index = 1;
    for (const auto& [file_name, file_path] : model_files)
    {
        add_model(file_name, new Model(file_path, file_name));
    }
}

void ResourceManager::initialize_predefined_materials()
{
    add_material("grid", new Material("grid", get_shader("GridShader")));
    add_material("line", new Material("line", get_shader("LineShader")));
    add_material("light", new Material("light", get_shader("LightShader")));
    add_material("particle", new Material("particle", get_shader("ParticleShader")));
    add_material("bounding-box", new Material("bounding-box", get_shader("GridShader")));
}

void ResourceManager::shutdown()
{
    for (const auto& shader : Shaders.all() | views::values)
    {
        glDeleteProgram(shader->get_object_id());
    }

    Shaders.clear();
    Textures.clear();
    Models.clear();
    Materials.clear();
}
