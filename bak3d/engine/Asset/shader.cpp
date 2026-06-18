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

#include "shader.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Core/logger.h"

using namespace std;

namespace
{
    bool check_compile_errors(unsigned int shader_id, const string& shader_type, const string& shader_name)
    {
        GLint success;
        GLchar infoLog[1024];
        if (shader_type != "PROGRAM")
        {
            glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader_id, 1024, nullptr, infoLog);
                B3D_LOG_ERROR("Shader compilation error of type: %s%s\n %s", shader_name.c_str(), shader_type.c_str(), infoLog);
            }
        }
        else
        {
            glGetProgramiv(shader_id, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader_id, 1024, nullptr, infoLog);
                B3D_LOG_ERROR("Shader linking error of type: %s%s\n %s", shader_name.c_str(), shader_type.c_str(), infoLog);
            }
        }

        return success == 0;
    }

    string resolve_includes(const string& source, const string& shader_dir)
    {
        string result;
        istringstream stream(source);
        string line;

        static const regex include_pattern(R"re(^\s*#include\s+"([^"]+)"\s*$)re");

        while (getline(stream, line))
        {
            // Strip carriage return for Windows line endings
            if (!line.empty() && line.back() == '\r')
            {
                line.pop_back();
            }

            if (smatch match; regex_match(line, match, include_pattern))
            {
                // Go one directory up from shader_dir
                auto parent_dir = shader_dir.substr(0, shader_dir.find_last_of("/\\"));
                string include_path = parent_dir + "/" + match[1].str();

                ifstream include_file(include_path);

                if (!include_file.is_open())
                {
                    B3D_LOG_ERROR("Could not open included shader file: %s", include_path.c_str());
                    result += line + "\n"; // leave line in place so the GLSL error is traceable
                    continue;
                }

                stringstream include_stream;
                include_stream << include_file.rdbuf();
                result += include_stream.str() + "\n";
            }
            else
            {
                result += line + "\n";
            }
        }

        return result;
    }

    // Read a single source file, resolve #include directives, return the final string
    string read_source(const string& path)
    {
        const ifstream file(path);
        if (!file.is_open())
        {
            B3D_LOG_ERROR("Shader file not found: %s", path.c_str());
            return {};
        }
        ostringstream stream_buffer;
        stream_buffer << file.rdbuf();
        const string shader_dir = filesystem::path(path).parent_path().string();

        return resolve_includes(stream_buffer.str(), shader_dir);
    }
}

Shader::Shader()
    : Asset(filesystem::absolute("shaders/LineShader.vert").string(), "LineShader")
    , m_compiled(false)
{
    ShaderStageMap shader_stage_sources;
    shader_stage_sources.emplace(GL_VERTEX_SHADER, filesystem::absolute("shaders/LineShader.vert").string());
    shader_stage_sources.emplace(GL_FRAGMENT_SHADER, filesystem::absolute("shaders/LineShader.frag").string());
}

Shader::Shader(const ShaderStageMap& shader_stage_sources, const std::string& shader_name)
    : Asset(shader_stage_sources.begin()->second, shader_name)
    , m_stages(shader_stage_sources)
    , m_compiled(false)
{
    build();
}

Shader::Shader(const Shader& other_shader) : Shader(other_shader.m_stages, other_shader.get_object_name()) {}

Shader::~Shader()
{
    B3D_LOG_INFO("Shader %s has been deleted.", m_object_name.c_str());
}

void Shader::build()
{
    bool any_error = false;

    m_object_id = glCreateProgram();

    // Compile every declared stage and attach it
    vector<GLuint> shader_stage_ids;
    shader_stage_ids.reserve(m_stages.size());

    for (const auto& [gl_stage, path] : m_stages)
    {
        const string source = read_source(path);
        if (source.empty())
        {
            any_error = true;
            continue;
        }

        const char* src_ptr = source.c_str();
        GLuint id = glCreateShader(gl_stage);
        glShaderSource(id, 1, &src_ptr, nullptr);
        glCompileShader(id);

        any_error |= check_compile_errors(id, get_shader_stage_name_file_extension(gl_stage), m_object_name);

        glAttachShader(m_object_id, id);
        shader_stage_ids.push_back(id);
    }

    glLinkProgram(m_object_id);
    any_error |= check_compile_errors(m_object_id, "PROGRAM", m_object_name);

    // Intermediate shader objects are no longer needed after linking
    for (const GLuint stage_id : shader_stage_ids)
    {
        glDeleteShader(stage_id);
    }

    m_compiled = !any_error;

    if (m_compiled)
    {
        B3D_LOG_INFO("Shader '%s' (ID %d) compiled.", m_object_name.c_str(), m_object_id);
    }
    else
    {
        B3D_LOG_ERROR("Shader '%s' failed. See above.", m_object_name.c_str());
    }
}

void Shader::use() const
{
    glUseProgram(m_object_id);
}

void Shader::unuse() const
{
    glUseProgram(0);
}

void Shader::dispatch_compute(const GLuint groups_x, const GLuint groups_y, const GLuint groups_z) const
{
    if (m_stages.contains(GL_COMPUTE_SHADER))
    {
        glDispatchCompute(groups_x, groups_y, groups_z);
    }
}

void Shader::set_bool(const string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(m_object_id, name.c_str()), static_cast<int>(value));
}

void Shader::set_uint(const string& name, unsigned int value) const
{
    glUniform1ui(glGetUniformLocation(m_object_id, name.c_str()), value); // Warning: DO NOT use for texture samplers. Use integer instead
}

void Shader::set_int(const string& name, int value) const
{
    glUniform1i(glGetUniformLocation(m_object_id, name.c_str()), value);
}

void Shader::set_float(const string& name, GLfloat value) const
{
	glUniform1f(glGetUniformLocation(m_object_id, name.c_str()), value);
}

void Shader::set_vec2(const string& name, const glm::vec2& value) const
{
    glUniform2fv(glGetUniformLocation(m_object_id, name.c_str()), 1, &value[0]);
}

void Shader::set_vec2(const string& name, float x, float y) const
{
    glUniform2f(glGetUniformLocation(m_object_id, name.c_str()), x, y);
}

void Shader::set_vec3(const string& name, const glm::vec3& value) const
{
    glUniform3fv(glGetUniformLocation(m_object_id, name.c_str()), 1, &value[0]);
}

void Shader::set_vec3(const string& name, float x, float y, float z) const
{
    glUniform3f(glGetUniformLocation(m_object_id, name.c_str()), x, y, z);
}

void Shader::set_vec4(const string& name, const glm::vec4& value) const
{
    glUniform4fv(glGetUniformLocation(m_object_id, name.c_str()), 1, &value[0]);
}

void Shader::set_vec4(const string& name, float x, float y, float z, float w) const
{
    glUniform4f(glGetUniformLocation(m_object_id, name.c_str()), x, y, z, w);
}

void Shader::set_mat2(const string& name, const glm::mat2& mat) const
{
    glUniformMatrix2fv(glGetUniformLocation(m_object_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::set_mat3(const string& name, const glm::mat3& mat) const
{
    glUniformMatrix3fv(glGetUniformLocation(m_object_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::set_mat4(const string& name, const glm::mat4& mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(m_object_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::set_float_array(const string& name, const float* values, const GLsizei count) const
{
    glUniform1fv(glGetUniformLocation(m_object_id, name.c_str()), count, values);
}

void Shader::set_int_array(const string& name, const int* values, const GLsizei count) const
{
    glUniform1iv(glGetUniformLocation(m_object_id, name.c_str()), count, values);
}

void Shader::set_vec2_array(const string& name, const glm::vec2* values, const GLsizei count) const
{
    glUniform2fv(glGetUniformLocation(m_object_id, name.c_str()), count, glm::value_ptr(values[0]));
}

void Shader::set_vec3_array(const string& name, const glm::vec3* values, const GLsizei count) const
{
    glUniform3fv(glGetUniformLocation(m_object_id, name.c_str()), count, glm::value_ptr(values[0]));
}

void Shader::set_vec4_array(const string& name, const glm::vec4* values, const GLsizei count) const
{
    glUniform4fv(glGetUniformLocation(m_object_id, name.c_str()), count, glm::value_ptr(values[0]));
}

void Shader::set_mat3_array(const string& name, const glm::mat3* values, const GLsizei count) const
{
    glUniformMatrix3fv(glGetUniformLocation(m_object_id, name.c_str()), count, GL_FALSE, glm::value_ptr(values[0]));
}

void Shader::set_mat4_array(const string& name, const glm::mat4* values, const GLsizei count) const
{
    glUniformMatrix4fv(glGetUniformLocation(m_object_id, name.c_str()), count, GL_FALSE, glm::value_ptr(values[0]));
}