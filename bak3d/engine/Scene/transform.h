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


// =====================================================================================
// Transform class was made by George Mavroeidis as a base class for entity orientation
// in space based on position, orientation and scaling. It contains a bunch of functions
// and data for updating the object in space, most importantly, using a model matrix.
// =====================================================================================

#pragma once

#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>

/*
 * Wrapper class for transform data, such as position, rotation and scale for both local and global contexts.
 */
class Transform
{
public:
    // Matrix Data
    void compute_model_matrix() { m_global_model_matrix = get_local_model_matrix(); m_is_dirty = false;}
    void compute_model_matrix(const glm::mat4& parent_global_model_matrix) { m_global_model_matrix = parent_global_model_matrix * m_global_model_matrix; m_is_dirty = false;}
    glm::mat4 get_global_model_matrix() const { return m_global_model_matrix; }

    // Local
    void set_local_position(const glm::vec3& new_position) { m_local_position = new_position; m_is_dirty = false; }
    const glm::vec3& get_local_position() const { return m_local_position; }
    
    void set_local_euler_rotation(const glm::vec3& new_euler_rotation) { m_local_euler_rotation = new_euler_rotation; m_is_dirty = false; }
    const glm::vec3& get_local_euler_rotation() const { return m_local_euler_rotation; }
    
    void set_local_scale(const glm::vec3& new_scale) { m_local_scale = new_scale; m_is_dirty = false; }
    const glm::vec3& get_local_scale() const { return m_local_scale; }

    // Global
    const glm::vec3& get_global_position() const { return m_global_model_matrix[3]; }
    const glm::vec3& get_global_scale() const { return { glm::length(get_right()), glm::length(get_up()), glm::length(get_back()) }; }
    const glm::vec3& get_right() const { return m_global_model_matrix[0]; }
    const glm::vec3& get_up() const { return m_global_model_matrix[1]; }
    const glm::vec3& get_back() const { return m_global_model_matrix[2]; }
    const glm::vec3& get_forward() const { return -m_global_model_matrix[2]; }

    bool is_dirty() const { return m_is_dirty; }

protected:
    const glm::mat4& get_local_model_matrix() const { return m_local_model_matrix; }
    void compute_local_model_matrix()
    {
        const glm::mat4 transform_x = glm::rotate(glm::mat4(1.0f), glm::radians(m_local_euler_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        const glm::mat4 transform_y = glm::rotate(glm::mat4(1.0f), glm::radians(m_local_euler_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        const glm::mat4 transform_z = glm::rotate(glm::mat4(1.0f), glm::radians(m_local_euler_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        const glm::mat4 rotation_matrix = transform_y * transform_x * transform_z;

        m_local_model_matrix = glm::translate(glm::mat4(1.0f), m_local_position) * rotation_matrix * glm::scale(glm::mat4(1.0f), m_local_scale);
    }
    
    glm::vec3 m_local_position = glm::vec3(0.0f);
    glm::vec3 m_local_euler_rotation = glm::vec3(0.0f);
    glm::vec3 m_local_scale = glm::vec3(1.0f);

    glm::mat4 m_global_model_matrix = glm::mat4(1.0f);
    glm::mat4 m_local_model_matrix = glm::mat4(1.0f);

    bool m_is_dirty = true;
};
