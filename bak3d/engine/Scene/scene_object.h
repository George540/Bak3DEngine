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
#include <glm/gtc/matrix_transform.hpp>

#include "Core/bak3d_object.h"

/*
 * Abstract class for declaring objects using position, orientation and scaling in 3D space.
 */
class SceneObject : public Bak3DObject
{
protected:
    glm::vec3 m_position = glm::vec3(0.0f);
    glm::vec3 m_euler_rotation = glm::vec3(0.0f, 1.0f, 0.0f); // up vector
    glm::vec3 m_scaling = glm::vec3(1.0f);

    glm::mat4 m_model_matrix = glm::mat4(1.0f);

public:
    SceneObject() : SceneObject(glm::vec3(0.0f, 0.0f, 0.0f), "Object") {}
    SceneObject(const glm::vec3 position, const std::string& name) : Bak3DObject(name), m_position(position) {}
    virtual ~SceneObject() override = 0;

    glm::vec3 get_position() const { return m_position; }
    glm::vec3 get_rotation() const { return m_euler_rotation; }
    glm::vec3 get_scaling() const { return m_scaling; }

    void set_position(const glm::vec3& position) { m_position = position; }
    void set_rotation(const glm::vec3& rotation) { m_euler_rotation = rotation; }
    void set_scaling(const glm::vec3& scaling) { m_scaling = scaling; }

    glm::mat4 get_model_matrix() const { return m_model_matrix; }
    void set_model_matrix(const glm::vec3& position, const glm::vec3& scaling, const glm::vec3& rotation_axis, const float rotation_angle_degrees)
    {
        m_position = position;
        m_scaling  = scaling;
        m_model_matrix = glm::mat4(1.0f);
        m_model_matrix = translate(m_model_matrix, m_position)
                        * rotate(m_model_matrix, glm::radians(rotation_angle_degrees), rotation_axis)
                        * scale(m_model_matrix,m_scaling);
    }

    virtual void update(float dt) = 0;
};

inline SceneObject::~SceneObject() = default;
