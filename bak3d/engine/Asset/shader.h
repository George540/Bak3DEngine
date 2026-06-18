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

#include <glad/glad.h> 
#include <glm/fwd.hpp>
#include <string>

#include "asset.h"
#include "Core/global_definitions.h"

/*
 * Shader class used to load shader files and building them to be used in the engine.
 * Contains setters for uniforms and other file storing data.
 */
class Shader : public Asset
{
private:
	void build();

	ShaderStageMap m_stages;

	bool m_compiled;
public:
	// Constructor
	Shader();
	Shader(const ShaderStageMap& shader_stage_sources, const std::string& shader_name);
	Shader(const Shader& other_shader);
	~Shader() override;

	// Core functions
	[[nodiscard]] bool is_shader_compiled() const { return m_compiled; }
	void use() const;
	void unuse() const;

	void dispatch_compute(GLuint groups_x, GLuint groups_y, GLuint groups_z) const;

	// Utility Functions
	void set_bool(const std::string& name, bool value) const;
	void set_uint(const std::string& name, unsigned int value) const;
	void set_int(const std::string& name, int value) const;
	void set_float(const std::string& name, GLfloat value) const;

	void set_vec2(const std::string& name, const glm::vec2& value) const;
	void set_vec2(const std::string& name, float x, float y) const;
	void set_vec3(const std::string& name, const glm::vec3& value) const;
	void set_vec3(const std::string& name, float x, float y, float z) const;
	void set_vec4(const std::string& name, const glm::vec4& value) const;
	void set_vec4(const std::string& name, float x, float y, float z, float w) const;
	void set_mat2(const std::string& name, const glm::mat2& mat) const;
	void set_mat3(const std::string& name, const glm::mat3& mat) const;
	void set_mat4(const std::string& name, const glm::mat4& mat) const;

	void set_float_array(const std::string& name, const float* values, GLsizei count) const;
	void set_int_array(const std::string& name, const int* values, GLsizei count) const;
	void set_vec2_array(const std::string& name, const glm::vec2* values, GLsizei count) const;
	void set_vec3_array(const std::string& name, const glm::vec3* values, GLsizei count) const;
	void set_vec4_array(const std::string& name, const glm::vec4* values, GLsizei count) const;
	void set_mat3_array(const std::string& name, const glm::mat3* values, GLsizei count) const;
	void set_mat4_array(const std::string& name, const glm::mat4* values, GLsizei count) const;
};
