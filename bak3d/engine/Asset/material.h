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

#include "asset.h"
#include "asset_definitions.h"
#include <unordered_map>
#include <string>

#include <glad/glad.h> 
#include <glm/fwd.hpp>
#include <glm/glm.hpp> 

#include "texture.h"

/*
 * Material class that sets parameters to be sent to a specific shader used by a renderable component.
 * Materials for now are created at runtime and are not a file asset.
 */
class Material : public Asset
{
    ShaderRef m_shader;

    // Single value-type uniforms
    std::unordered_map<std::string, bool> m_uniform_flags;
    std::unordered_map<std::string, GLint> m_int_uniforms;
    std::unordered_map<std::string, GLuint> m_uint_uniforms;
    std::unordered_map<std::string, GLfloat> m_float_uniforms;

    // Multi vector-type uniforms
    std::unordered_map<std::string, glm::vec2> m_vec2_uniforms;
    std::unordered_map<std::string, glm::vec3> m_vec3_uniforms;
    std::unordered_map<std::string, glm::vec4> m_vec4_uniforms;
    std::unordered_map<std::string, glm::mat2> m_mat2_uniforms;
    std::unordered_map<std::string, glm::mat3> m_mat3_uniforms;
    std::unordered_map<std::string, glm::mat4> m_mat4_uniforms;

public:
    Material() = default;
    // Materials aren't loaded from disk, so path is just the name
    explicit Material(const std::string& path, const std::string& file_name, const ShaderRef& shader);
    Material(const Material& other_material);

    bool is_loaded() const { return m_shader.is_valid(); }

    void set_shader(const ShaderRef& shader) { m_shader = shader; }
    ShaderRef get_shader() const { return m_shader; }

    void load_from_file(const std::string& path);
    void save_to_file() const;
    void apply();
    void bind_textures_cache();

    std::unordered_map<aiTextureType, std::string> m_texture_names;

    // Setters
    void set_bool(const std::string& key_name, const bool value) { m_uniform_flags[key_name] = value; }
    void set_uint(const std::string& key_name, const unsigned int value) { m_uint_uniforms[key_name] = value; }
    void set_int(const std::string& key_name, const int value) { m_int_uniforms[key_name] = value; }
    void set_float(const std::string& key_name, const float value) { m_float_uniforms[key_name] = value; }

    void set_vec2(const std::string& key_name, const glm::vec2& value) { m_vec2_uniforms[key_name] = value; }
    void set_vec3(const std::string& key_name, const glm::vec3& value) { m_vec3_uniforms[key_name] = value; }
    void set_vec4(const std::string& key_name, const glm::vec4& value) { m_vec4_uniforms[key_name] = value; }

    void set_mat2(const std::string& key_name, const glm::mat2& value) { m_mat2_uniforms[key_name] = value; }
    void set_mat3(const std::string& key_name, const glm::mat3& value) { m_mat3_uniforms[key_name] = value; }
    void set_mat4(const std::string& key_name, const glm::mat4& value) { m_mat4_uniforms[key_name] = value; }

    // Getters (only done on the material level. Not the shader)
    bool has_texture_of_type(const aiTextureType& texture_type) const { return m_texture_names.contains(texture_type); }
    std::string get_texture_by_type(const aiTextureType& texture_type);
    void set_texture_by_type(const aiTextureType& texture_type, const std::string& texture_name);
    void remove_texture_by_type(const aiTextureType& key_name);
    bool has_uniform(const std::string& key_name) const;
    
    bool get_bool(const std::string& key_name) { return m_uniform_flags[key_name]; }
    GLuint get_uint(const std::string& key_name) { return m_uint_uniforms[key_name]; }
    GLuint get_int(const std::string& key_name) { return m_int_uniforms[key_name]; }
    float get_float(const std::string& key_name) { return m_float_uniforms[key_name]; }

    glm::vec2 get_vec2(const std::string& key_name) { return m_vec2_uniforms[key_name]; }
    glm::vec3 get_vec3(const std::string& key_name) { return m_vec3_uniforms[key_name]; }
    glm::vec4 get_vec4(const std::string& key_name) { return m_vec4_uniforms[key_name]; }

    glm::mat2 get_mat2(const std::string& key_name) { return m_mat2_uniforms[key_name]; }
    glm::mat3 get_mat3(const std::string& key_name) { return m_mat3_uniforms[key_name]; }
    glm::mat4 get_mat4(const std::string& key_name) { return m_mat4_uniforms[key_name]; }
};