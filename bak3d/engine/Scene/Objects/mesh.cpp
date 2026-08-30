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

#include "mesh.h"

#include <utility>

#include "Asset/mesh_data.h"
#include "Asset/resource_manager.h"

using namespace std;

Mesh::Mesh(const glm::vec3 position, const std::string& name, const MaterialRef& material, const std::string& mesh_data_name) :
    RenderableObject(material, position, name)
{
    object_type = SceneObjectType::Mesh;
    m_mesh_slot = make_mesh_slot(ResourceManager::get_mesh(mesh_data_name));
}

void Mesh::update(float dt)
{
    RenderableObject::update(dt);
}

void Mesh::draw() const
{
    RenderableObject::draw();
}
