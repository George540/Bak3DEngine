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

#include <assimp/postprocess.h>
#define STB_IMAGE_IMPLEMENTATION    
#include "stb_image.h"

#include <iostream>
#include <filesystem>

#include "model.h"

#include <set>

#include "file_loader.h"
#include "mesh_data.h"
#include "Asset/resource_manager.h"
#include "Core/logger.h"
#include "Scene/scene.h"

using namespace std;

namespace
{
	set<Vertex> m_unique_vertices;
	std::set<Edge> m_unique_edges;
	std::set<Face> m_unique_faces;
}

Model::Model(const string& path, const std::string& file_name) :
	Asset(path, file_name)
{
	m_object_name = FileLoader::get_name_from_filename(m_object_name);
	load_model(path);

	// Gather number of vertices but discard set storages to save on space.
	m_num_vertices = m_unique_vertices.size();
	m_num_edges = m_unique_edges.size();
	m_num_faces = m_unique_faces.size();
	m_unique_vertices.clear();
	m_unique_edges.clear();
	m_unique_faces.clear();

	m_current_material_slot = make_material_slot();
	set_current_material(m_object_name + "_material");
}

Model::~Model()
{
	// Free texture data
	m_textures_cache.clear();
	B3D_LOG_INFO("Texture data of model %s has been cleared", m_file_name.c_str());

	m_current_material_slot = nullptr;
	
	m_mesh_data.clear();
	B3D_LOG_INFO("Model %s mesh data have been safely deleted", m_file_name.c_str());
}
 
void Model::load_model(string const& path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate
														| aiProcess_GenSmoothNormals
														| aiProcess_FlipUVs
														| aiProcess_CalcTangentSpace
														| aiProcess_JoinIdenticalVertices
														| aiProcess_SortByPType
														| aiProcess_FindInvalidData);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) //if it's not zero
	{
		B3D_LOG_ERROR("Assimp error: %s", importer.GetErrorString());
		return;
	}

	B3D_LOG_INFO("Loading model with name %s", m_file_name.c_str());

	m_root_node = process_node(scene->mRootNode, scene);
}

// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
unique_ptr<ModelNode> Model::process_node(aiNode* node, const aiScene* scene)
{
	auto model_node = make_unique<ModelNode>();
	model_node->name = node->mName.C_Str();

	const aiMatrix4x4& transform = node->mTransformation;
	model_node->local_transform = glm::mat4(
		transform.a1, transform.b1, transform.c1, transform.d1,
		transform.a2, transform.b2, transform.c2, transform.d2,
		transform.a3, transform.b3, transform.c3, transform.d3,
		transform.a4, transform.b4, transform.c4, transform.d4);

	// Handle all cases of referenced meshes, whether is itself or children
	for (GLuint i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* ai_mesh = scene->mMeshes[node->mMeshes[i]];
		model_node->meshes.push_back(process_mesh(ai_mesh, scene, node->mMeshes[i]));
	}

	for (GLuint i = 0; i < node->mNumChildren; ++i)
	{
		model_node->children.push_back(process_node(node->mChildren[i], scene));
	}

	return model_node;
}

MeshRef Model::process_mesh(aiMesh* mesh, const aiScene* scene, const int mesh_index)
{
	// data to fill
	vector<Vertex> vertices;
	vector<GLuint> indices;
	vector<string> textures;

	// walk through each of the mesh's vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex{};
		glm::vec3 vector; // temp glm::vec3 vector for parsing data to Assimp using the vertex payload

		// positions
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.position = vector;

		// normals
		if (mesh->HasNormals())
		{
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.normal = vector;
		}

		// texture coordinates
		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			glm::vec2 vec;
			
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.tex_coords = vec;

			// tangent
			vector.x = mesh->mTangents[i].x;
			vector.y = mesh->mTangents[i].y;
			vector.z = mesh->mTangents[i].z;
			vertex.tangent = vector;

			// bitangent
			vector.x = mesh->mBitangents[i].x;
			vector.y = mesh->mBitangents[i].y;
			vector.z = mesh->mBitangents[i].z;
			vertex.bitangent = vector;
		}
		else
		{
			vertex.tex_coords = glm::vec2(0.0f, 0.0f);
		}

		if (scene->mNumMaterials > mesh->mMaterialIndex)
		{
			const auto& mat = scene->mMaterials[mesh->mMaterialIndex];
			aiColor4D diffuse;
			aiColor4D specular;
			aiColor4D normals;

			// Check if diffuse texture exists
			if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
			{
				vertex.color = glm::vec4(diffuse.r, diffuse.g, diffuse.b, diffuse.a);
			}
			else
			{
				vertex.color = glm::vec4(1.0f, 0.75f, 0.796f, 1.0f);
			}

			// Check if specular texture exists
			if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_SPECULAR, &specular))
			{
				vertex.color = glm::vec4(specular.r, specular.g, specular.b, specular.a);
			}
		}

		m_unique_vertices.insert(vertex);
		vertices.push_back(vertex);
	}

	// now walk through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		auto face = mesh->mFaces[i];

		Face unique_face(face);

		// Insert into set (returns a pair; second is true if newly inserted)
		auto result = m_unique_faces.insert(unique_face);

		// Retrieve all indices of the face and store them in the indices vector
		for (unsigned int j = 0; j < face.mNumIndices; ++j)
		{
			assert(face.mIndices[j] < mesh->mNumVertices);
			indices.push_back(face.mIndices[j]);
			GLuint v1 = face.mIndices[j];
			GLuint v2 = face.mIndices[(j + 1) % face.mNumIndices];
			m_unique_edges.insert(Edge(v1, v2));
		}
	}

	// If model is to be loaded with all its textures and materials individually, set this to true.
	// We want textures and materials to be loaded independent of the model at the moment.
	if (false)
	{
		// Process materials
		auto material = scene->mMaterials[mesh->mMaterialIndex];

		// 1. diffuse maps
		load_material_textures(material, aiTextureType_DIFFUSE);
		// 2. specular maps
		load_material_textures(material, aiTextureType_SPECULAR);
		// 3. normal maps
		load_material_textures(material, aiTextureType_NORMALS);
	}

	// Return a mesh object created from the extracted mesh data
	const string mesh_key = m_object_name + "_mesh_" + to_string(mesh_index);
	ResourceManager::add_mesh(mesh_key, new MeshData(move(vertices), move(indices), mesh->mName.C_Str()));
	return ResourceManager::get_mesh(mesh_key);
}

void Model::load_material_textures(aiMaterial* mat, aiTextureType type)
{
	for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i)
	{
		aiString filename;
		mat->GetTexture(type, i, &filename);

		if (!m_textures_cache.contains(type)) // only one texture for each texture type at a time
		{
			string path = m_directory + '/' + filename.C_Str();
			string texture_file_name = string(filename.C_Str());
			Texture2D* texture = new Texture2D(path, texture_file_name);
			auto texture_name = texture_file_name.substr(0, texture_file_name.find('.'));
			auto texture_key = format("{}.{}",m_object_name, texture_name);
			ResourceManager::add_texture(texture_key, texture);
			m_textures_cache[type] = texture;
		}
	}
}

void Model::set_current_material(const std::string& material_name)
{
	// Write into the shared slot. All meshes see this instantly
	*m_current_material_slot = ResourceManager::get_material(material_name);
}
