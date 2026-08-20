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

#include <string>

#include "asset.h"
#include "asset_definitions.h"
#include "material.h"
#include "resource_map.h"
#include "shader.h"

class Shader;
class Material;
class Model;
class Texture2D;

/*
 * Static class for managing loaded rendering resources such as shaders, textures, materials, meshes
 * and models to be used in the engine.
 */
class ResourceManager
{
public:
    // resource storage
    static ResourceMap<Shader> Shaders;
    static ResourceMap<Texture2D> Textures;
    static ResourceMap<Material> Materials;
    static ResourceMap<Model> Models;
    static ResourceMap<PrimitiveData> Meshes;

    static void initialize();

    static void reload_shaders();

    static void add_shader(const std::string& name, Shader* shader) { Shaders.insert_or_swap(name, shader, [](const Shader* s) { return s != nullptr && s->is_shader_compiled(); }); }
    static void add_texture(const std::string& name, Texture2D* texture) { Textures.insert_or_swap(name, texture, [](const Texture2D* t) { return t != nullptr; }); }
    static void add_material(const std::string& name, Material* material) { Materials.insert_or_swap(name, material, [](const Material* m) { return m != nullptr; }); }
    static void add_model(const std::string& name, Model* model) { Models.insert_or_swap(name, model, [](const Model* m) { return m != nullptr; }); }
    static void add_mesh(const std::string& name, PrimitiveData* mesh) { Meshes.insert_or_swap(name, mesh, [](const PrimitiveData* m) { return m != nullptr; }); }

    static ShaderRef get_shader(const std::string& name) { return Shaders.get(name); }
    static TextureRef get_texture(const std::string& name) { return Textures.get(name); }
    static MaterialRef get_material(const std::string& name) { return Materials.get(name); }
    static ModelRef get_model(const std::string& name) { return Models.get(name); }
    static MeshRef get_mesh(const std::string& name) { return Meshes.get(name); }
    
    static void shutdown();
private:
    static void initialize_shaders();
    static void initialize_predefined_textures();
    static void initialize_models();
    static void initialize_predefined_meshes();
    static void initialize_predefined_materials();
    
    ResourceManager() = default;
};