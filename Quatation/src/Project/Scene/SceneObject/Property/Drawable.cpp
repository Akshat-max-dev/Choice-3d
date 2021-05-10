#include "Drawable.h"

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include<glm/gtx/quaternion.hpp>

#include "Transform.h"

namespace choice
{
	struct DumpableMeshData
	{
		std::vector<float> Vertices;
		std::vector<uint32_t> Indices;
		uint32_t MaterialIndex;
	};

	struct DumpableMaterialData
	{
		std::string MaterialName;
		Texture2DData DiffuseMap;
		Texture2DData NormalMap;
		float Roughness;
		float Metallic;
		glm::vec4 Color;
	};

	struct DumpableJointData
	{
		std::string Name;
		std::string ParentName;
		glm::mat4 InvBindMatrix;
	};

	struct DumpableArmatureData
	{
		std::vector<DumpableJointData> Joints;
		glm::mat4 transform;
		//TODO : Animation
	};

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

	glm::mat4 toTranform(const glm::vec3 position, const glm::quat rotation, const glm::vec3 scale)
	{
		return glm::translate(glm::mat4(1.0f), position) *
			glm::toMat4(rotation) * glm::scale(glm::mat4(1.0f), scale);
	}

	enum MESH_ATTRIBUTE_TYPE
	{
		POSITION = 0, NORMAL = 1, TEXCOORDS = 2, TANGENTS = 3, COUNT = 4
	};

	static char const* MESH_ATTRIBUTES[MESH_ATTRIBUTE_TYPE::COUNT] = {
		"POSITION", "NORMAL", "TEXCOORD_0", "TANGENT"
	};

	//Load All Joints Node
	void loadJointNode(const cgltf_node* node, DumpableArmatureData& armaturedata, uint32_t& callCount)
	{
		armaturedata.Joints[callCount].Name = node->name;
		armaturedata.Joints[callCount].ParentName = callCount ? node->parent->name : "";

		//Visit The Children Joint Nodes
		for (auto i = 0; i < node->children_count; i++)
		{
			const auto* child = node->children[i];
			loadJointNode(child, armaturedata, ++callCount);
		}
	}

	//Load Node
	void loadNode(const cgltf_node* node, std::vector<DumpableMeshData>* meshdata,
		std::vector<DumpableMaterialData>* materialdata, std::vector<DumpableArmatureData>* armaturedata,
		const std::string& dstDirectory)
	{
		if (node->mesh)
		{
			auto& primitive = node->mesh->primitives[0];

			uint32_t MaterialIndex;
			bool loadMaterial = true;

			//Load Material
			auto& material = primitive.material;

			//Check For Existing Material
			for (uint32_t i = 0; i < materialdata->size(); i++)
			{
				if (materialdata->at(i).MaterialName == material->name)
				{
					MaterialIndex = i;
					loadMaterial = false;
					break;
				}
			}

			if (material && loadMaterial)
			{
				DumpableMaterialData data;
				data.MaterialName = material->name;
				data.Roughness = material->pbr_metallic_roughness.roughness_factor;
				data.Metallic = material->pbr_metallic_roughness.metallic_factor;
				data.Color.r = material->pbr_metallic_roughness.base_color_factor[0];
				data.Color.g = material->pbr_metallic_roughness.base_color_factor[1];
				data.Color.b = material->pbr_metallic_roughness.base_color_factor[2];
				data.Color.a = material->pbr_metallic_roughness.base_color_factor[3];
				const auto* diffusemap = material->pbr_metallic_roughness.base_color_texture.texture;
				if (diffusemap)
				{
					data.DiffuseMap.Source = CompressTexture((void*)diffusemap, dstDirectory,
						BlockCompressionFormat::BC1, true);
					data.DiffuseMap.magFilter = (uint32_t)diffusemap->sampler->mag_filter;
					data.DiffuseMap.minFilter = (uint32_t)diffusemap->sampler->min_filter;
					data.DiffuseMap.wrapS = (uint32_t)diffusemap->sampler->wrap_s;
					data.DiffuseMap.wrapT = (uint32_t)diffusemap->sampler->wrap_t;
				}
				else { data.DiffuseMap = {}; }
				const auto* normalmap = material->normal_texture.texture;
				if (normalmap)
				{
					data.NormalMap.Source = CompressTexture((void*)normalmap, dstDirectory,
						BlockCompressionFormat::BC5, true);
					data.NormalMap.magFilter = (uint32_t)normalmap->sampler->mag_filter;
					data.NormalMap.minFilter = (uint32_t)normalmap->sampler->min_filter;
					data.NormalMap.wrapS = (uint32_t)normalmap->sampler->wrap_s;
					data.NormalMap.wrapT = (uint32_t)normalmap->sampler->wrap_t;
				}
				else { data.NormalMap = {}; }
				materialdata->push_back(data);
				MaterialIndex = (uint32_t)materialdata->size() - 1;
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

			//Filling Vertex Buffer
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

			data.MaterialIndex = MaterialIndex;

			meshdata->push_back(data);
		}

		if (node->skin) //Load Skin Info
		{
			//Extract Per-Joint Inverse Bind Matrix
			DumpableArmatureData data;

			data.Joints.resize(node->skin->joints_count);

			const cgltf_accessor* accessor = node->skin->inverse_bind_matrices;
			const auto* view = accessor->buffer_view;
			const float* buffer = reinterpret_cast<float*>((char*)view->buffer->data + view->offset);

			auto componentcount = component_count(accessor->type);
			for (auto idx = 0; idx < accessor->count; idx++)
			{
				float t[16] = {
					buffer[idx * componentcount + 0],  buffer[idx * componentcount + 1],
					buffer[idx * componentcount + 2],  buffer[idx * componentcount + 3],

					buffer[idx * componentcount + 4],  buffer[idx * componentcount + 5],
					buffer[idx * componentcount + 6],  buffer[idx * componentcount + 7],

					buffer[idx * componentcount + 8],  buffer[idx * componentcount + 9],
					buffer[idx * componentcount + 10], buffer[idx * componentcount + 11],

					buffer[idx * componentcount + 12], buffer[idx * componentcount + 13],
					buffer[idx * componentcount + 14], buffer[idx * componentcount + 15]
				};

				cgltfMatrixtoMat4(t, data.Joints[idx].InvBindMatrix);
			}

			//Extract Armature TRS
			glm::vec3 t = node->skin->joints[0]->parent->has_translation ?
				glm::vec3(node->skin->joints[0]->parent->translation[0], node->skin->joints[0]->parent->translation[1],
					node->skin->joints[0]->parent->translation[2]) :
				glm::vec3(0.0f, 0.0f, 0.0f);

			glm::quat r = node->skin->joints[0]->parent->has_rotation ?
				glm::quat(node->skin->joints[0]->parent->rotation[0], node->skin->joints[0]->parent->rotation[1],
					node->skin->joints[0]->parent->rotation[2], node->skin->joints[0]->parent->rotation[3]) :
				glm::quat(0.0f, 0.0f, 0.0f, 1.0f);

			glm::vec3 s = node->skin->joints[0]->parent->has_scale ?
				glm::vec3(node->skin->joints[0]->parent->scale[0], node->skin->joints[0]->parent->scale[1],
					node->skin->joints[0]->parent->scale[2]) :
				glm::vec3(0.0f, 0.0f, 0.0f);

			data.transform = toTranform(t, glm::quat(0.0f, 0.0f, 0.0f, 1.0f), s);

			uint32_t callCount = 0;
			loadJointNode(node->skin->joints[0], data, callCount);

			armaturedata->push_back(data);
		}

		//Children Nodes
		for (auto i = 0; i < node->children_count; i++)
		{
			const auto* child = node->children[i];
			loadNode(child, meshdata, materialdata, armaturedata, dstDirectory);
		}
	}

	//Write The Model In Engine Format
	const std::string DumpDrawable(const std::string& srcFile, const std::string& dstDirectory, DrawableType type)
	{
		if (type == DrawableType::MODEL)
		{
			std::string temp = srcFile.substr(srcFile.find_last_of('\\') + 1, srcFile.size());
			std::string dstFile = dstDirectory + "\\" + temp.substr(0, temp.find_last_of('.')) + ".cmodel";
			std::string dstMaterialFile = dstDirectory + "\\" + temp.substr(0, temp.find_last_of('.')) + ".cmaterial";

			if (ghc::filesystem::exists(dstFile)) { return dstFile; }

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
			std::vector<DumpableArmatureData>* armaturedata = new std::vector<DumpableArmatureData>();

			cgltf_scene* scene = data->scene;
			auto nodescount = scene->nodes_count;
			for (auto i = 0; i < nodescount; i++)
			{
				const auto* node = scene->nodes[i];
				loadNode(node, meshdata, materialdata, armaturedata, dstDirectory);
			}

			std::ofstream cmaterial(dstMaterialFile, std::ios::out | std::ios::binary);

			uint32_t materialsize = (uint32_t)materialdata->size();
			cmaterial.write((char*)&materialsize, sizeof(materialsize));
			for (auto& material : *materialdata)
			{
				uint32_t materialnamesize = (uint32_t)material.MaterialName.size();
				cmaterial.write((char*)&materialnamesize, sizeof(materialnamesize));
				cmaterial.write((char*)material.MaterialName.data(), materialnamesize);

				cmaterial.write((char*)&material.Roughness, sizeof(material.Roughness));
				cmaterial.write((char*)&material.Metallic, sizeof(material.Metallic));

				cmaterial.write((char*)&material.Color.r, sizeof(material.Color.r));
				cmaterial.write((char*)&material.Color.g, sizeof(material.Color.g));
				cmaterial.write((char*)&material.Color.b, sizeof(material.Color.b));
				cmaterial.write((char*)&material.Color.a, sizeof(material.Color.a));

				uint32_t diffusemapnamesize = (uint32_t)material.DiffuseMap.Source.size();
				cmaterial.write((char*)&diffusemapnamesize, sizeof(diffusemapnamesize));
				if (diffusemapnamesize)
				{
					cmaterial.write((char*)material.DiffuseMap.Source.data(), diffusemapnamesize);

					cmaterial.write((char*)&material.DiffuseMap.magFilter, sizeof(material.DiffuseMap.magFilter));
					cmaterial.write((char*)&material.DiffuseMap.minFilter, sizeof(material.DiffuseMap.minFilter));
					cmaterial.write((char*)&material.DiffuseMap.wrapS, sizeof(material.DiffuseMap.wrapS));
					cmaterial.write((char*)&material.DiffuseMap.wrapT, sizeof(material.DiffuseMap.wrapT));
				}

				uint32_t normalmapnamesize = (uint32_t)material.NormalMap.Source.size();
				cmaterial.write((char*)&normalmapnamesize, sizeof(normalmapnamesize));
				if (normalmapnamesize)
				{
					cmaterial.write((char*)material.NormalMap.Source.data(), normalmapnamesize);

					cmaterial.write((char*)&material.NormalMap.magFilter, sizeof(material.NormalMap.magFilter));
					cmaterial.write((char*)&material.NormalMap.minFilter, sizeof(material.NormalMap.minFilter));
					cmaterial.write((char*)&material.NormalMap.wrapS, sizeof(material.NormalMap.wrapS));
					cmaterial.write((char*)&material.NormalMap.wrapT, sizeof(material.NormalMap.wrapT));
				}
			}

			delete materialdata;
			cmaterial.close();

			std::ofstream cmodel(dstFile, std::ios::out | std::ios::binary);
			if (cmodel.fail())
			{
				std::cout << "Cannot Dump Model Data" << std::endl;
				cmodel.close();
				return {};
			}

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

				cmodel.write((char*)&mesh.MaterialIndex, sizeof(mesh.MaterialIndex));
			}

			delete meshdata;

			uint32_t armaturesize = (uint32_t)armaturedata->size();
			cmodel.write((char*)&armaturesize, sizeof(armaturesize));
			if (armaturesize)
			{
				uint32_t jointssize = (uint32_t)armaturedata->at(0).Joints.size();
				cmodel.write((char*)&jointssize, sizeof(jointssize));
				for (auto& joint : armaturedata->at(0).Joints)
				{
					uint32_t jointnamesize = (uint32_t)joint.Name.size();
					cmodel.write((char*)&jointnamesize, sizeof(jointnamesize));
					cmodel.write((char*)joint.Name.data(), jointnamesize);

					uint32_t parentjointnamesize = (uint32_t)joint.ParentName.size();
					cmodel.write((char*)&parentjointnamesize, sizeof(parentjointnamesize));
					if (parentjointnamesize)
					{
						cmodel.write((char*)joint.ParentName.data(), parentjointnamesize);
					}

					cmodel.write((char*)glm::value_ptr(joint.InvBindMatrix), sizeof(joint.InvBindMatrix));
				}

				cmodel.write((char*)glm::value_ptr(armaturedata->at(0).transform), sizeof(armaturedata->at(0).transform));
			}

			delete armaturedata;

			cmodel.close();

			cgltf_free(data);

			return dstFile;
		}

		return {};
	}

	Drawable* LoadDrawable(const std::string& info, DrawableType type, bool loadMaterials)
	{
		Drawable* drawable = {};

		switch (type)
		{
		case DrawableType::CUBE: //Create Cube
			drawable = new Drawable(info, type);

			if (loadMaterials)
			{
				drawable->GetMaterials().resize(1);
				for (auto& material : drawable->GetMaterials())
				{
					material = new Material();
					material->Name = "Material 0";
				}
			}

			drawable->GetMeshes().resize(1);
			for (auto& mesh : drawable->GetMeshes())
			{
				float vertices[] = {
					-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
					 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
					 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
					 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
					-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
					-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

					-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
					 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
					 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
					 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
					-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
					-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

					-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
					-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
					-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
					-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
					-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
					-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

					 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
					 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
					 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
					 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
					 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
					 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

					-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
					 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
					 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
					 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
					-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
					-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

					-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
					 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
					 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
					 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
					-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
					-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
				};

				mesh.first = new VertexArray();
				mesh.first->VertexBuffer(vertices, sizeof(vertices), "332");
				mesh.first->IndexBuffer(nullptr, 0);

				mesh.second = 0;

				drawable->GetBoundingBox() = CalculateBoundingBox(vertices, 36 * 8, 8);
			}
			break;
		case DrawableType::SPHERE: //Create Sphere
			drawable = new Drawable(info, DrawableType::SPHERE);

			if (loadMaterials)
			{
				drawable->GetMaterials().resize(1);
				for (auto& material : drawable->GetMaterials())
				{
					material = new Material();
					material->Name = "Material 0";
				}
			}

			drawable->GetMeshes().resize(1);
			for (auto& mesh : drawable->GetMeshes())
			{
				std::vector<glm::vec3> positions;
				std::vector<glm::vec3> normals;
				std::vector<glm::vec2> texcoords;

				const uint32_t X_SEGMENTS = 64;
				const uint32_t Y_SEGMENTS = 64;
				const float PI = 3.14159265359f;

				for (uint32_t y = 0; y <= Y_SEGMENTS; ++y)
				{
					for (uint32_t x = 0; x <= X_SEGMENTS; ++x)
					{
						float xSegment = (float)x / (float)X_SEGMENTS;
						float ySegment = (float)y / (float)Y_SEGMENTS;
						float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
						float yPos = std::cos(ySegment * PI);
						float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

						positions.push_back({ xPos, yPos, zPos });
						normals.push_back({ xPos, yPos, zPos });
						texcoords.push_back({ xSegment, ySegment });
					}
				}

				std::vector<uint32_t> indices;

				bool oddRow = false;
				for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
				{
					if (!oddRow) // even rows: y == 0, y == 2; and so on
					{
						for (uint32_t x = 0; x <= X_SEGMENTS; ++x)
						{
							indices.push_back(y * (X_SEGMENTS + 1) + x);
							indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
						}
					}
					else
					{
						for (int x = X_SEGMENTS; x >= 0; --x)
						{
							indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
							indices.push_back(y * (X_SEGMENTS + 1) + x);
						}
					}
					oddRow = !oddRow;
				}

				std::vector<float> vertices;
				for (uint32_t i = 0; i < positions.size(); ++i)
				{
					vertices.push_back(positions[i].x);
					vertices.push_back(positions[i].y);
					vertices.push_back(positions[i].z);
					if (normals.size() > 0)
					{
						vertices.push_back(normals[i].x);
						vertices.push_back(normals[i].y);
						vertices.push_back(normals[i].z);
					}
					if (texcoords.size() > 0)
					{
						vertices.push_back(texcoords[i].x);
						vertices.push_back(texcoords[i].y);
					}
				}

				mesh.first = new VertexArray();
				mesh.first->VertexBuffer(vertices.data(), vertices.size() * sizeof(float), "332");
				mesh.first->IndexBuffer(indices.data(), static_cast<uint32_t>(indices.size()));

				mesh.second = 0;

				drawable->GetBoundingBox() = CalculateBoundingBox(vertices.data(), (uint32_t)vertices.size(), 8);
			}
			break;
		case DrawableType::MODEL: //Load Model
			//Extract Name From File
			std::string temp = info.substr(info.find_last_of('\\') + 1, info.size());

			drawable = new Drawable(temp.substr(0, temp.find_last_of('.')), DrawableType::MODEL);

			//Read Materials Data
			if (loadMaterials)
			{
				std::ifstream cmaterial(info.substr(0, info.find_last_of('.')) + ".cmaterial", std::ios::in | std::ios::binary);

				uint32_t materialssize;
				cmaterial.read((char*)&materialssize, sizeof(materialssize));
				drawable->GetMaterials().resize(materialssize);
				for (auto& material : drawable->GetMaterials())
				{
					material = new Material();

					uint32_t namesize;
					cmaterial.read((char*)&namesize, sizeof(namesize));
					material->Name.resize(namesize);
					cmaterial.read((char*)material->Name.data(), namesize);

					cmaterial.read((char*)&material->Roughness, sizeof(material->Roughness));
					cmaterial.read((char*)&material->Metallic, sizeof(material->Metallic));

					cmaterial.read((char*)&material->Color.r, sizeof(material->Color.r));
					cmaterial.read((char*)&material->Color.g, sizeof(material->Color.g));
					cmaterial.read((char*)&material->Color.b, sizeof(material->Color.b));
					cmaterial.read((char*)&material->Color.a, sizeof(material->Color.a));

					//Diffuse Map
					Texture2DData* diffusemapdata = new Texture2DData();
					if (LoadMaterialsData(cmaterial, diffusemapdata))
					{
						material->DiffuseMap.first = true;
						material->DiffuseMap.second.first = new Texture2D(LoadTexture2D(*diffusemapdata));
						material->DiffuseMap.second.second = diffusemapdata;
					}
					else { delete diffusemapdata; }

					//Normal Map
					Texture2DData* normalmapdata = new Texture2DData();
					if (LoadMaterialsData(cmaterial, normalmapdata))
					{
						material->NormalMap.first = true;
						material->NormalMap.second.first = new Texture2D(LoadTexture2D(*normalmapdata));
						material->NormalMap.second.second = normalmapdata;
					}
					else { delete normalmapdata; }
				}

				cmaterial.close();
			}

			std::ifstream cmodel(info, std::ios::in | std::ios::binary);
			if (cmodel.fail())
			{
				std::cout << "Error Loading Model" << std::endl;
				cmodel.close();
				break;
			}

			//Read Number Of Mesh In The Model
			uint32_t meshsize;
			cmodel.read((char*)&meshsize, sizeof(meshsize));
			drawable->GetMeshes().resize(meshsize);

			std::vector<BoundingBox> meshboundingboxes;

			//Read Geometry Data
			for (auto& mesh : drawable->GetMeshes())
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

				uint32_t materialindex;
				cmodel.read((char*)&materialindex, sizeof(materialindex));

				mesh.first = new VertexArray();
				mesh.first->VertexBuffer(vertices.data(), vertices.size() * sizeof(float), "3323");
				mesh.first->IndexBuffer(indices.data(), (uint32_t)indices.size());
				mesh.second = materialindex;

				meshboundingboxes.push_back(CalculateBoundingBox(vertices.data(), (uint32_t)vertices.size(), 11));
			}

			float minX = std::numeric_limits<float>::max();
			float minY = minX;
			float minZ = minY;
			float maxX = std::numeric_limits<float>::min();
			float maxY = maxX;
			float maxZ = maxY;

			for (auto& b : meshboundingboxes)
			{
				minX = b.Min.x < minX ? b.Min.x : minX;
				minY = b.Min.y < minY ? b.Min.y : minY;
				minZ = b.Min.z < minZ ? b.Min.z : minZ;

				maxX = b.Max.x > maxX ? b.Max.x : maxX;
				maxY = b.Max.y > maxY ? b.Max.y : maxY;
				maxZ = b.Max.z > maxZ ? b.Max.z : maxZ;
			}

			drawable->GetBoundingBox().Min = { minX, minY, minZ };
			drawable->GetBoundingBox().Max = { maxX, maxY, maxZ };

			//Read Animation Data
			uint32_t armaturesize;
			cmodel.read((char*)&armaturesize, sizeof(armaturesize));
			if (armaturesize)
			{
				Animation* anim = new Animation();

				uint32_t jointssize;
				cmodel.read((char*)&jointssize, sizeof(jointssize));
				anim->GetSkeleton()->GetJointsData().resize(jointssize);
				for (auto& joint : anim->GetSkeleton()->GetJointsData())
				{
					joint = new Joint();

					uint32_t jointnamesize;
					cmodel.read((char*)&jointnamesize, sizeof(jointnamesize));
					joint->Name.resize(jointnamesize);
					cmodel.read((char*)joint->Name.data(), jointnamesize);

					uint32_t parentnamesize;
					cmodel.read((char*)&parentnamesize, sizeof(parentnamesize));
					if (parentnamesize)
					{
						std::string parentname;
						parentname.resize(parentnamesize);
						cmodel.read((char*)parentname.data(), parentnamesize);

						for (auto& search_joint : anim->GetSkeleton()->GetJointsData())
						{
							if (search_joint->Name == parentname)
							{
								joint->Parent = search_joint;
								break;
							}
						}
					}

					cmodel.read((char*)glm::value_ptr(joint->InvBindMatrix), sizeof(joint->InvBindMatrix));
				}

				glm::mat4 t;
				cmodel.read((char*)glm::value_ptr(t), sizeof(t));
				anim->GetSkeleton()->SetArmatureTransform(t);

				drawable->SetAnimation(anim);
			}

			cmodel.close();
			break;
		}

		return drawable;
	}

	Drawable::Drawable(const std::string& name, DrawableType type)
		:mName(name), mDrawableType(type)
	{
		mAnimation = {};
	}

	Drawable::~Drawable()
	{
		for (auto& material : mMaterials) { if (material)delete material; }
		for (auto& mesh : mMeshes) { if (mesh.first)delete mesh.first; }
		if (mAnimation) { delete mAnimation; }
	}
}