#include "Model.h"

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include <glm/glm.hpp>

namespace choice
{
	Model::~Model()
	{
		for (auto& mesh : Meshes)
		{
			delete mesh.first;
		}
	}

	static cgltf_size component_size(cgltf_component_type component_tpe)
	{
		switch (component_tpe)
		{
		case cgltf_component_type_r_8:
		case cgltf_component_type_r_8u:
			return 1;
		case cgltf_component_type_r_16:
		case cgltf_component_type_r_16u:
			return 2;
		case cgltf_component_type_r_32u:
		case cgltf_component_type_r_32f:
			return 4;
		case cgltf_component_type_invalid:
		default:
			return 0;
		}
	}

	static cgltf_size component_count(cgltf_type type) {
		switch (type) {
		case cgltf_type_scalar:
			return 1;
		case cgltf_type_vec2:
			return 2;
		case cgltf_type_vec3:
			return 3;
		case cgltf_type_vec4:
		case cgltf_type_mat2:
			return 4;
		case cgltf_type_mat3:
			return 9;
		case cgltf_type_mat4:
			return 16;
		case cgltf_type_invalid:
			std::cout << "Invalid Type" << std::endl;
			return 0;
		}
		return 0;
	}

	enum MESH_ATTRIBUTE_TYPE
	{
		POSITION = 0, NORMAL = 1, TEXCOORDS = 2, TANGENTS = 3, COUNT = 4
	};

	static char const* MESH_ATTRIBUTES[MESH_ATTRIBUTE_TYPE::COUNT] = {
		"POSITION", "NORMAL", "TEXCOORD_0", "TANGENT"
	};

	void loadNode(const cgltf_node* node, Model* model)
	{
		if (node->mesh)
		{
			auto& primitive = node->mesh->primitives[0];

			std::vector<glm::vec3> fullMeshData[MESH_ATTRIBUTE_TYPE::COUNT];

			bool attribFound = false;
			int attribFoundIndex = 0;

			for (auto i = 0; i < MESH_ATTRIBUTE_TYPE::COUNT; i++)
			{
				const char* attribute = MESH_ATTRIBUTES[i];
				for (auto a = 0; a < primitive.attributes_count; a++)
				{
					const auto& primAttrib = primitive.attributes[a];
					attribFound = strcmp(primAttrib.name, attribute) == 0;
					if (attribFound) { attribFoundIndex = a; break; }
				}

				if (!attribFound)
				{
					if (strcmp(attribute, MESH_ATTRIBUTES[MESH_ATTRIBUTE_TYPE::POSITION]) == 0)
					{
						std::cout << "Positions Can't Be Empty" << std::endl;
						return;
					}
					else
					{
						if (attribute == MESH_ATTRIBUTES[MESH_ATTRIBUTE_TYPE::TEXCOORDS])
						{
							fullMeshData[i].resize((fullMeshData[i - 1].size() / 3) * 2);
						}
						fullMeshData[i].resize(fullMeshData[i - 1].size());
						for (auto& x : fullMeshData[i])
						{
							x = { 0.0f, 0.0f, 0.0f };
						}
					}
					continue;
				}

				auto& meshData = fullMeshData[i];
				const cgltf_accessor* accessor = node->mesh->primitives->attributes[attribFoundIndex].data;
				const auto* view = accessor->buffer_view;

				const float* data = reinterpret_cast<float*>((char*)view->buffer->data + view->offset);
				
				auto componentcount = component_count(accessor->type);

				for (auto idx = 0; idx < accessor->count; idx++)
				{
					if (i != MESH_ATTRIBUTE_TYPE::TEXCOORDS)
					{
						meshData.push_back({ data[idx * componentcount + 0],
						data[idx * componentcount + 1] , data[idx * componentcount + 2] });
					}
					else
					{
						meshData.push_back({ data[idx * componentcount + 0],
						data[idx * componentcount + 1] , 0.0f });
					}
					
				}
			}

			//Index Buffer
			const cgltf_accessor* accessor = primitive.indices;
			const auto* view = accessor->buffer_view;

			auto componentsize = component_size(accessor->component_type);

			std::vector<uint32_t> indices;

			switch (componentsize)
			{
			case 4:
			{
				const auto* source = reinterpret_cast<uint32_t const*>((char*)view->buffer->data + view->offset);
				for (auto i = 0; i < accessor->count; i++)
				{
					indices.push_back(source[i]);
				}
			}
				break;
			case 2:
			{
				const auto* source = reinterpret_cast<uint16_t const*>((char*)view->buffer->data + view->offset);
				for (auto i = 0; i < accessor->count; i++)
				{
					indices.push_back(source[i]);
				}
			}
				break;
			default:
				std::cout << "Mesh Index Buffer Needs To Be Either 4 or 2 Bytes!" << std::endl;
				return;
			}

			std::vector<float> vertices;

			for (auto i = 0; i < fullMeshData[0].size(); i++)
			{
				vertices.push_back(fullMeshData[0][i].x);
				vertices.push_back(fullMeshData[0][i].y);
				vertices.push_back(fullMeshData[0][i].z);

				vertices.push_back(fullMeshData[1][i].x);
				vertices.push_back(fullMeshData[1][i].y);
				vertices.push_back(fullMeshData[1][i].z);

				vertices.push_back(fullMeshData[2][i].x);
				vertices.push_back(fullMeshData[2][i].y);

				vertices.push_back(fullMeshData[3][i].x);
				vertices.push_back(fullMeshData[3][i].y);
				vertices.push_back(fullMeshData[3][i].z);
			}

			std::pair<VertexArray*, uint32_t> mesh;
			mesh.first = new VertexArray();
			mesh.second = 0;

			mesh.first->VertexBuffer(vertices.data(), vertices.size() * sizeof(float), "3 3 2 3");
			mesh.first->IndexBuffer(indices.data(), (uint32_t)indices.size());

			model->Meshes.push_back(mesh);
		}

		for (auto i = 0; i < node->children_count; i++)
		{
			const auto* child = node->children[i];
			loadNode(child, model);
		}
	}

	Model* LoadModel(const std::string& srcFile)
	{
		cgltf_options options = {};
		cgltf_data* data = nullptr;
		cgltf_result result = cgltf_parse_file(&options, srcFile.c_str(), &data);
		if (result != cgltf_result_success)
		{
			std::cout << "Error Loading Model" << std::endl;
			return nullptr;
		}

		result = cgltf_load_buffers(&options, data, srcFile.c_str());
		if (result != cgltf_result_success)
		{
			cgltf_free(data);
			std::cout << "Error Loading Buffers" << std::endl;
			return nullptr;
		}

		Model* model = new Model();

		cgltf_scene* scene = data->scene;
		auto nodescount = scene->nodes_count;
		for (auto i = 0; i < nodescount; i++)
		{
			const auto* node = scene->nodes[i];
			loadNode(node, model);
		}
		
		cgltf_free(data);
		return model;
	}
}