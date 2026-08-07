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

#include "Asset/material.h"
#include "Core/global_definitions.h"
#include "Core/core_interface.h"
#include "Renderer/buffer.h"
#include "Renderer/vertex_array.h"
#include "Scene/scene_object.h"

/*
 * Base primitive object class for drawable engine scene objects.
 * Contains data for initializing, binding, drawing and cleaning up.
 */
class RenderableObject : public SceneObject, public IRenderable
{
protected:
    VertexArray* m_vao;
    VertexBuffer* m_vbo;
    ElementBuffer* m_ebo;

    MaterialSlot m_material_slot;

    bool m_visible;

    void apply_material() const { if (has_material()) (*m_material_slot)->apply(); }
public:
    RenderableObject(const MaterialRef& material, const glm::vec3 position, const std::string& name);
    ~RenderableObject() override;

    void update(float dt) override;
    void draw() const override;

    void set_material(const MaterialRef& material) const { *m_material_slot = material; } // Standalone objects swap their own slot
    void set_material_slot(const MaterialSlot& slot) { m_material_slot = slot; } // Adopt a shared slot from a parent model
    bool has_material() const { return m_material_slot && *m_material_slot; }
    MaterialRef get_material() const { return *m_material_slot; }
    void set_visible(bool visible) { m_visible = visible; }
    bool is_visible() const { return m_visible; }
};

/*
 * Instanced version of the object class, containing an instanced buffer.
 * Used for drawing multiple instances of the same object in the same draw call.
 */
class InstancedRenderableObject : public RenderableObject
{
protected:
    InstanceBuffer* m_ibo;
public:
    InstancedRenderableObject(const MaterialRef& material, const std::string& name);
    ~InstancedRenderableObject() override;

    void draw() const override;
};
