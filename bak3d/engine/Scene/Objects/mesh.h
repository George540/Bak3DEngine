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
#include <vector>

#include "Core/global_definitions.h"
#include "Scene/Objects/renderable_object.h"

/*
 * Renderable object made out of vertices and edges to form a shape out of surfaces.
 */
class Mesh : public RenderableObject
{
public:
    // mesh Data
    std::vector<Vertex> m_vertices;
    std::vector<GLuint> m_indices;

    // constructor
    Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, const std::string& name);
    ~Mesh() override = default;
    void update(float dt) override;
    void draw() const override;
};

/*
 * Extension of Mesh that handles multiple instances of the same mesh archetype
 */
class InstancedMesh : public Mesh
{
protected:
    InstanceBuffer* m_ibo;
    int m_num_instances;
public:
    InstancedMesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, const std::string& name, const int initial_num_instances) : Mesh(vertices, indices, name), m_ibo(nullptr), m_num_instances(initial_num_instances) {}
    ~InstancedMesh() override = default;

    void draw() const override { RenderableObject::draw(); };
    int get_num_instances() const { return m_num_instances; }
};