#include "Model.h"

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include <glm/glm.hpp>

#include "OpenGL/Texture.h"

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

	void loadNode(const cgltf_node* node, std::vector<DumpableMeshData>* meshdata,
		std::vector<DumpableMaterialData>* materialdata, const std::string& dstDirectory)
	{
		if (node->mesh)
		{
			auto& primitive = node->mesh->primitives[0];

			//Load Material
			auto& material = primitive.material;
			if (material)
			{
				DumpableMaterialData data;
				const auto* diffusemap = material->pbr_metallic_roughness.base_color_texture.texture;
				data.DiffuseMap = CompressTexture((void*)diffusemap, dstDirectory,
					BlockCompressionFormat::BC1, true);
				const auto* normalmap = material->normal_texture.texture;
				if (normalmap)
				{
					data.NormalMap = CompressTexture((void*)normalmap, dstDirectory,
						BlockCompressionFormat::BC5, true);
				}
				else { data.NormalMap = {}; }
				materialdata->push_back(data);
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

			DumpableMeshData data;

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
					data.Indices.push_back(source[i]);
				}
			}
				break;
			case 2:
			{
				const auto* source = reinterpret_cast<uint16_t const*>((char*)view->buffer->data + view->offset);
				for (auto i = 0; i < accessor->count; i++)
				{
					data.Indices.push_back(source[i]);
				}
			}
				break;
			default:
				std::cout << "Mesh Index Buffer Needs To Be Either 4 or 2 Bytes!" << std::endl;
				return;
			}

			for (auto i = 0; i < fullMeshData[0].size(); i++)
			{
				data.Vertices.push_back(fullMeshData[0][i].x);
				data.Vertices.push_back(fullMeshData[0][i].y);
				data.Vertices.push_back(fullMeshData[0][i].z);

				data.Vertices.push_back(fullMeshData[1][i].x);
				data.Vertices.push_back(fullMeshData[1][i].y);
				data.Vertices.push_back(fullMeshData[1][i].z);

				data.Vertices.push_back(fullMeshData[2][i].x);
				data.Vertices.push_back(fullMeshData[2][i].y);

				data.Vertices.push_back(fullMeshData[3][i].x);
				data.Vertices.push_back(fullMeshData[3][i].y);
				data.Vertices.push_back(fullMeshData[3][i].z);
			}

			meshdata->push_back(data);
		}

		for (auto i = 0; i < node->children_count; i++)
		{
			const auto* child = node->children[i];
			loadNode(child, meshdata, materialdata, dstDirectory);
		}
	}

	Model* LoadModel(const std::string& srcFile)
	{
		std::ifstream cmodel(srcFile, std::ios::in | std::ios::binary);
		if (cmodel.fail() || cmodel.bad())
		{
			std::cout << "Error Loading Model" << std::endl;
			cmodel.close();
			return nullptr;
		}

		Model* model = new Model();

		std::string temp = srcFile.substr(srcFile.find_last_of('\\') + 1, srcFile.size());
		model->Name = temp.substr(0, temp.find_last_of('.'));

		uint32_t meshsize;
		cmodel.read((char*)&meshsize, sizeof(meshsize));
		model->Meshes.resize(meshsize);

		for (auto& mesh : model->Meshes)
		{
			uint32_t verticessize;
			cmodel.read((char*)&verticessize, sizeof(verticessize));
			std::vector<float> vertices;
			vertices.resize(verticessize);
			cmodel.read((char*)vertices.data(), verticessize * sizeof(float));

			uint32_t indicessize;
			cmodel.read((char*)&indicessize, sizeof(indicessize));
			std::vector<uint32_t> indices;
			indices.resize(indicessize);
			cmodel.read((char*)indices.data(), indicessize * sizeof(uint32_t));
		}

		cmodel.close();
		return model;
	}

	const std::string DumpModel(const std::string& srcFile, const std::string& dstDirectory)
	{
		std::string temp = srcFile.substr(srcFile.find_last_of('\\') + 1, srcFile.size());
		std::string dstFile = dstDirectory + "\\" + temp.substr(0, temp.find_last_of('.')) + ".cmodel";

		//To Check If Model Already Exists
		std::ifstream check(dstFile, std::ios::in | std::ios::binary);
		if (check.is_open()) { check.close(); return dstFile; }
		check.close();

		cgltf_options options = {};
		cgltf_data* data = nullptr;
		cgltf_result result = cgltf_parse_file(&options, srcFile.c_str(), &data);
		if (result != cgltf_result_success)
		{
			std::cout << "Error Loading Model" << std::endl;
			return {};
		}

		result = cgltf_load_buffers(&options, data, srcFile.c_str());
		if (result != cgltf_result_success)
		{
			cgltf_free(data);
			std::cout << "Error Loading Buffers" << std::endl;
			return {};
		}

		std::vector<DumpableMeshData>* meshdata = new std::vector<DumpableMeshData>();
		std::vector<DumpableMaterialData>* materialdata = new std::vector<DumpableMaterialData>();

		cgltf_scene* scene = data->scene;
		auto nodescount = scene->nodes_count;
		for (auto i = 0; i < nodescount; i++)
		{
			const auto* node = scene->nodes[i];
			loadNode(node, meshdata, materialdata, dstDirectory);
		}

		std::ofstream cmodel(dstFile, std::ios::out | std::ios::binary);
		if (cmodel.fail())
		{
			std::cout << "Cannot Dump Model Data" << std::endl;
			cmodel.close();
			return {};
		}

		uint32_t materialsize = (uint32_t)materialdata->size();
		cmodel.write((char*)&materialsize, sizeof(materialsize));
		for (auto& material : *materialdata)
		{
			uint32_t diffusemapnamesize = (uint32_t)material.DiffuseMap.size();
			cmodel.write((char*)&diffusemapnamesize, sizeof(diffusemapnamesize));
			cmodel.write((char*)material.DiffuseMap.data(), diffusemapnamesize);

			uint32_t normalmapnamesize = (uint32_t)material.NormalMap.size();
			cmodel.write((char*)&normalmapnamesize, sizeof(normalmapnamesize));
			cmodel.write((char*)material.NormalMap.data(), normalmapnamesize);
		}

		delete materialdata;

		uint32_t meshsize = (uint32_t)meshdata->size();
		cmodel.write((char*)&meshsize, sizeof(meshsize));
		for (auto& mesh : *meshdata)
		{
			uint32_t verticessize = (uint32_t)mesh.Vertices.size();
			cmodel.write((char*)&verticessize, sizeof(verticessize));
			cmodel.write((char*)mesh.Vertices.data(), verticessize * sizeof(float));

			uint32_t indicessize = (uint32_t)mesh.Indices.size();
			cmodel.write((char*)&indicessize, sizeof(indicessize));
			cmodel.write((char*)mesh.Indices.data(), indicessize * sizeof(uint32_t));
		}

		delete meshdata;

		cgltf_free(data);

		return dstFile;
	}

}