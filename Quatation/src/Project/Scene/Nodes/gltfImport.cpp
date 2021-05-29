#include "gltfImport.h"

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#define GLM_ENABLE_EXPERIMENTAL
#include<glm/gtx/quaternion.hpp>

#include "ReflectionData.h"
#include "XML/XMLFile.h"
#include "Choice.h"

namespace choice
{
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

	void cgltfMatrixtoMat4(const cgltf_float data[16], glm::mat4& transform)
	{
		transform[0][0] = data[0];  transform[1][0] = data[4];	transform[2][0] = data[8];  transform[3][0] = data[12];
		transform[0][1] = data[1];  transform[1][1] = data[5];	transform[2][1] = data[9];	transform[3][1] = data[13];
		transform[0][2] = data[2];	transform[1][2] = data[6];	transform[2][2] = data[10]; transform[3][2] = data[14];
		transform[0][3] = data[3];  transform[1][3] = data[7];	transform[2][3] = data[11];	transform[3][3] = data[15];
	}

	enum MESH_ATTRIBUTE_TYPE
	{
		POSITION = 0, NORMAL = 1, TEXCOORDS = 2, COUNT = 3
	};

	static char const* MESH_ATTRIBUTES[MESH_ATTRIBUTE_TYPE::COUNT] = {
		"POSITION", "NORMAL", "TEXCOORD_0"
	};

	void loadcgltfNodeTransform(const cgltf_node* node, Transform* transform)
	{
		if (node->has_matrix)
		{
			glm::mat4 t;
			cgltfMatrixtoMat4(node->matrix, t);
			DecomposeTransform(t, transform->Position, transform->Rotation, transform->Scale);
		}
		else
		{
			if (node->has_translation)
				transform->Position = { node->translation[0], node->translation[1], node->translation[2] };

			if (node->has_rotation)
				transform->Rotation = { node->rotation[0], node->rotation[1], node->rotation[2] };

			if (node->has_scale)
				transform->Scale = { node->scale[0], node->scale[1], node->scale[2] };
		}
	}

	void loadcgltfNode(const cgltf_node* cgltfnode, Node*& node, Node* parent, XMLFile* cmesh)
	{
		if (!cgltfnode->mesh)
		{
			node = new Node();
			node->Id = ++global::NodeCounter;
			node->node_data_type = NODE_DATA_TYPE::NONE;
			node->Name = cgltfnode->name;
			loadcgltfNodeTransform(cgltfnode, node->NodeTransform);
		}
		else if (cgltfnode->mesh)
		{
			Mesh* mesh = new Mesh();
			mesh->mesh_type = MESH_TYPE::IMPORTED;
			mesh->node_data_type = NODE_DATA_TYPE::MESH;
			mesh->Name = cgltfnode->name;
			mesh->Id = ++global::NodeCounter;
			loadcgltfNodeTransform(cgltfnode, mesh->NodeTransform);

			mesh->primitives.resize(cgltfnode->mesh->primitives_count);

			std::vector<BoundingBox> primitvebb;

			for (auto prim = 0; prim < cgltfnode->mesh->primitives_count; prim++)
			{
				mesh->primitives[prim] = new Primitive();

				auto& primitive = cgltfnode->mesh->primitives[prim];

				//Load Material
				auto& cgltfmaterial = primitive.material;

				if (cgltfmaterial)
				{
					mesh->primitives[prim]->material = new Material();
					Material* material = mesh->primitives[prim]->material;
					material->Name = cgltfmaterial->name;

					auto* materialBuffer = global::GlobalReflectionData.UniformBuffers["Material"];

					int* hasalbedoemap = materialBuffer->MemberData<int>("Material.HasAlbedoMap", material->Data);
					int* hasnormalmap = materialBuffer->MemberData<int>("Material.HasNormalMap", material->Data);
					int* hasaomap = materialBuffer->MemberData<int>("Material.HasAoMap", material->Data);
					float* roughness = materialBuffer->MemberData<float>("Material.Roughness", material->Data);
					float* metallic = materialBuffer->MemberData<float>("Material.Metallic", material->Data);
					glm::vec3* color = materialBuffer->MemberData<glm::vec3>("Material.Color", material->Data);

					*color = { cgltfmaterial->pbr_metallic_roughness.base_color_factor[0],
							   cgltfmaterial->pbr_metallic_roughness.base_color_factor[1],
							   cgltfmaterial->pbr_metallic_roughness.base_color_factor[2] };

					*roughness = cgltfmaterial->pbr_metallic_roughness.roughness_factor;
					*metallic = cgltfmaterial->pbr_metallic_roughness.metallic_factor;
					
					const auto* albedomap = cgltfmaterial->pbr_metallic_roughness.base_color_texture.texture;
					if (albedomap)
					{
						*hasalbedoemap = 1;
						std::string filepath = CompressTexture((void*)albedomap, 
							BlockCompressionFormat::BC1, true);

						material->TextureMaps[TEXTURE_MAP_TYPE::ALBEDO]->filepath = filepath;
						material->TextureMaps[TEXTURE_MAP_TYPE::ALBEDO]->texture = new Texture2D(LoadTexture2D(filepath));
					}

					const auto* normalmap = cgltfmaterial->normal_texture.texture;
					if (normalmap)
					{
						*hasnormalmap = 1;
						std::string filepath = CompressTexture((void*)normalmap,
							BlockCompressionFormat::BC5, true);

						material->TextureMaps.insert({ TEXTURE_MAP_TYPE::NORMAL, new TextureMap() });

						material->TextureMaps[TEXTURE_MAP_TYPE::NORMAL]->filepath = filepath;
						material->TextureMaps[TEXTURE_MAP_TYPE::NORMAL]->texture = new Texture2D(LoadTexture2D(filepath));
					}

					const auto* occlusionmap = cgltfmaterial->occlusion_texture.texture;
					if (occlusionmap)
					{
						*hasaomap = 1;
						std::string filepath = CompressTexture((void*)occlusionmap,
							BlockCompressionFormat::BC4, true);

						material->TextureMaps.insert({ TEXTURE_MAP_TYPE::AMBIENT_OCCLUSION, new TextureMap() });

						material->TextureMaps[TEXTURE_MAP_TYPE::AMBIENT_OCCLUSION]->filepath = filepath;
						material->TextureMaps[TEXTURE_MAP_TYPE::AMBIENT_OCCLUSION]->texture = new Texture2D(LoadTexture2D(filepath));
					}
				}

				//Load Mesh
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
							choiceassert(0);
						}
						else
						{
							fullMeshData[i].resize(fullMeshData[i - 1].size());
							for (auto& x : fullMeshData[i])
							{
								x = { 0.0f, 0.0f, 0.0f };
							}
						}
						continue;
					}

					auto& meshData = fullMeshData[i];
					const cgltf_accessor* accessor = cgltfnode->mesh->primitives->attributes[attribFoundIndex].data;
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

				std::vector<uint32_t> indices;

				//Index Buffer
				const cgltf_accessor* accessor = primitive.indices;
				const auto* view = accessor->buffer_view;

				auto componentsize = component_size(accessor->component_type);

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
					choiceassert(0);
				}

				std::vector<float> vertices;

				//Filling Vertex Buffer
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
				}

				std::string meshname = mesh->Name;
				meshname.erase(std::remove(meshname.begin(), meshname.end(), ' '), meshname.end());

				meshname += std::to_string(prim);

				std::vector<char> buffer;
				auto* element = cmesh->GetFile()->NewElement(meshname.c_str());

				buffer.resize(vertices.size() * sizeof(float));
				memcpy(buffer.data(), (char*)vertices.data(), buffer.size());
				element->InsertEndChild(WriteBinaryData(cmesh->GetFile(), "Vertices", buffer));

				buffer.clear();
				buffer.resize(indices.size() * sizeof(uint32_t));
				memcpy(buffer.data(), (char*)indices.data(), buffer.size());
				element->InsertEndChild(WriteBinaryData(cmesh->GetFile(), "Indices", buffer));

				cmesh->GetFile()->InsertEndChild(element);
				buffer.clear();

				mesh->primitives[prim]->vertexarray = new VertexArray();
				mesh->primitives[prim]->vertexarray->VertexBuffer(vertices.data(), vertices.size() * sizeof(float), "332");
				mesh->primitives[prim]->vertexarray->IndexBuffer(indices.data(), (uint32_t)indices.size());

				primitvebb.push_back(CalculateBoundingBox(vertices.data(), (uint32_t)vertices.size(), 8));
			}

			mesh->boundingbox = CalculateBoundingBox(nullptr, 0, 0);

			for (auto& bb : primitvebb)
			{
				mesh->boundingbox.Min.x = bb.Min.x < mesh->boundingbox.Min.x ? bb.Min.x : mesh->boundingbox.Min.x;
				mesh->boundingbox.Min.y = bb.Min.y < mesh->boundingbox.Min.y ? bb.Min.y : mesh->boundingbox.Min.y;
				mesh->boundingbox.Min.z = bb.Min.z < mesh->boundingbox.Min.z ? bb.Min.z : mesh->boundingbox.Min.z;

				mesh->boundingbox.Max.x = bb.Max.x > mesh->boundingbox.Max.x ? bb.Max.x : mesh->boundingbox.Max.x;
				mesh->boundingbox.Max.y = bb.Max.y > mesh->boundingbox.Max.y ? bb.Max.y : mesh->boundingbox.Max.y;
				mesh->boundingbox.Max.z = bb.Max.z > mesh->boundingbox.Max.z ? bb.Max.z : mesh->boundingbox.Max.z;
			}

			node = mesh;
		}

		if (node)
		{
			node->Children.resize(cgltfnode->children_count);
			node->Parent = parent;
		}

		//Load Children
		for (auto i = 0; i < cgltfnode->children_count; i++)
		{
			const auto* cgltfchild = cgltfnode->children[i];
			loadcgltfNode(cgltfchild, node->Children[i], node, cmesh);
		}
	}

	bool ImportGLTF(const std::string& srcFile, Node* root)
	{
		cgltf_options options = {};
		cgltf_data* data = nullptr;
		cgltf_result result = cgltf_parse_file(&options, srcFile.c_str(), &data);
		if (result != cgltf_result_success)
		{
			std::cout << "Error Loading Model" << std::endl;
			return false;
		}

		result = cgltf_load_buffers(&options, data, srcFile.c_str());
		if (result != cgltf_result_success)
		{
			cgltf_free(data);
			std::cout << "Error Loading Buffers" << std::endl;
			return false;
		}

		XMLFile* cmesh = new XMLFile();

		std::string name = ghc::filesystem::path(srcFile).stem().string();

		std::string dstMeshFile = global::ActiveSceneDir + "Assets\\" + name + std::to_string(root->Id) + ".cmesh";

		root->Name = name;
		root->node_data_type = NODE_DATA_TYPE::NONE;

		cgltf_scene* scene = data->scene;
		auto nodescount = scene->nodes_count;

		root->Children.resize(nodescount);

		for (auto i = 0; i < nodescount; i++)
		{
			const auto* node = scene->nodes[i];
			loadcgltfNode(node, root->Children[i], root, cmesh);
		}

		cmesh->Save(dstMeshFile);

		delete cmesh;

		cgltf_free(data);

		//TODO : Check If A Texture Is Shared Among Other Materials Than Load It Only Once

		return true;
	}
}