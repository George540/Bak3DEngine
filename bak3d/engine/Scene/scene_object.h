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

#include "transform.h"
#include "Core/bak3d_object.h"
#include "Core/global_definitions.h"

/*
 * Abstract class for declaring objects using position, orientation and scaling in 3D space.
 */
class SceneObject : public Bak3DObject
{
public:
    Transform transform;

    SceneObject* parent = nullptr;
    std::vector<std::unique_ptr<SceneObject>> children;

    SceneObjectType object_type;

    bool m_is_dirty = true;
    bool is_active = true;

    SceneObject() : SceneObject(glm::vec3(0.0f, 0.0f, 0.0f), "SceneObject") {}
    SceneObject(const glm::vec3 position, const std::string& name) : Bak3DObject(name) { transform.set_local_position(position); }
    ~SceneObject() override = default;

    // If no arguments, default constructor is called instead for the called class.
    template<typename T>
    T* add_child(std::unique_ptr<T> child)
    {
        static_assert(std::is_base_of_v<SceneObject, T>, "Incorrect child class type. SceneObject-derived type is required.");

        T* child_raw = child.get();
        child_raw->parent = this;
        child_raw->transform.mark_dirty();
        children.emplace_back(std::move(child));

        return child_raw;
    }

    void update_self_and_children()
    {
        if (transform.is_dirty())
        {
            force_update_self_and_children();
            return;
        }

        for (auto&& child : children)
        {
            child->update_self_and_children();
        }
    }

    void force_update_self_and_children()
    {
        if (parent)
        {
            transform.compute_model_matrix(parent->transform.get_global_model_matrix());
        }
        else
        {
            transform.compute_model_matrix();
        }

        for (auto&& child : children)
        {
            child->force_update_self_and_children();
        }
    }

    SceneObjectType get_object_type() const { return object_type; }
    void set_object_type(const SceneObjectType type) { object_type = type; }

    [[nodiscard]]
    bool is_dirty() const { return m_is_dirty; }
    void mark_dirty() { m_is_dirty = true; }

    void clear_dirty()
    {
        m_is_dirty = false;
    }

    virtual void update(float dt)
    {
        if (!is_active)
        {
            return;
        }

        update_self_and_children();
    }
};
