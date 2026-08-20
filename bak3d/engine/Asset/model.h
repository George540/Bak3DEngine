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

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include <string>
#include <vector>

#include "mesh_data.h"
#include "Asset/asset.h"
#include "Asset/asset_definitions.h"
#include "Asset/texture.h"
#include "Scene/Objects/mesh.h"


struct ModelNode
{
	std::string name;
	glm::mat4 local_transform{ 1.0f };
	std::vector<MeshRef> meshes;
	std::vector<std::unique_ptr<ModelNode>> children;
};

/*
 * An asset class that stores information about a model loaded from a file using assimp.
 * Uses stored mesh and material data to instantiate a set of meshes in the scene.
 */
class Model : public Asset
{
public:
	glm::mat4 local_rotation;

	// constructor, expects a filepath to a 3D model.

	Model() = default;
	Model(const std::string& path, const std::string& file_name);
	~Model() override;

	const ModelNode* get_root_node() const { return m_root_node.get(); }

	void set_current_material(const std::string& material_name);
	MaterialRef get_current_material() const { return *m_current_material_slot; }

	// Model stats
	std::vector<MeshData*> get_all_mesh_data() const { return m_mesh_data; }
	GLuint get_vertices() const { return m_num_vertices; }
	GLuint get_unique_edges() const { return m_num_edges; }
	GLuint get_faces() const { return m_num_faces; }
private:
	// loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
	void load_model(std::string const& path);
	// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	std::unique_ptr<ModelNode> process_node(aiNode* node, const aiScene* scene);
	MeshRef process_mesh(aiMesh* mesh, const aiScene* scene, int mesh_index);

	void load_material_textures(aiMaterial* mat, aiTextureType type);

	std::unique_ptr<ModelNode> m_root_node;

	// model data
	std::vector<MeshData*> m_mesh_data;
	std::unordered_map<aiTextureType, Texture2D*> m_textures_cache;
	MaterialSlot m_current_material_slot;

	// model stats
	GLuint m_num_vertices;
	GLuint m_num_edges;
	GLuint m_num_faces;
};