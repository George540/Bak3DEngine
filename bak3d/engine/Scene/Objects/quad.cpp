#include "quad.h"

#include "Asset/mesh_data.h"
#include "Asset/resource_manager.h"

Quad::Quad() : RenderableObject(nullptr, glm::vec3(0.0f, 0.0f, 0.0f), "Quad")
{
    m_mesh_slot = make_mesh_slot(ResourceManager::get_mesh("Quad"));
}

void Quad::draw() const
{
    (*m_mesh_slot)->draw();
}
