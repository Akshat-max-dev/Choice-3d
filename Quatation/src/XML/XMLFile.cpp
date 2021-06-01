#include "XMLFile.h"

#include "Error.h"

namespace choice
{
	XMLFile::XMLFile()
	{
		mFile = new tinyxml2::XMLDocument();
	}

	XMLFile::~XMLFile()
	{
		delete mFile;
	}

	tinyxml2::XMLElement* WriteBinaryData(tinyxml2::XMLDocument* file, const char* name, std::vector<char>& data)
	{
		tinyxml2::XMLElement* binaryelement = file->NewElement(name);

		//Encode Binary To Base64
		std::string encodeddata = base64_encode(reinterpret_cast<const uint8_t*>(data.data()), data.size());

		binaryelement->SetText(encodeddata.c_str());

		return binaryelement;
	}

	tinyxml2::XMLElement* WriteMaterial(tinyxml2::XMLDocument* file, Material* material)
	{
		tinyxml2::XMLElement* materialelement = file->NewElement("Material");
	
		tinyxml2::XMLElement* name = file->NewElement("Name");
		name->SetText(material->Name.c_str());
		materialelement->InsertEndChild(name);

		materialelement->InsertEndChild(WriteBinaryData(file, "Data", material->Data));

		for (auto& texturemap : material->TextureMaps)
		{
			tinyxml2::XMLElement* texturemapelement = file->NewElement("TextureMap");
			texturemapelement->SetAttribute("Type", static_cast<int>(texturemap.first));
			texturemapelement->SetText(texturemap.second->filepath.c_str());
			materialelement->InsertEndChild(texturemapelement);
		}

		return materialelement;
	}

	void XMLFile::WriteNode(Node* node)
	{
		tinyxml2::XMLElement* rootelement = mFile->NewElement(("Node" + std::to_string(node->Id)).c_str());
		rootelement->SetAttribute("Type", static_cast<int>(node->node_data_type));
		mFile->InsertEndChild(rootelement);

		auto addelement = [&](const char* name, const auto& data) {
			tinyxml2::XMLElement* element = mFile->NewElement(name);
			element->SetText(data);
			rootelement->InsertEndChild(element);
		};

		addelement("Name", node->Name.c_str());

		switch (node->node_data_type)
		{
		case NODE_DATA_TYPE::MESH:
			{
				Mesh* mesh = static_cast<Mesh*>(node);
				addelement("InternalType", static_cast<int>(mesh->mesh_type));

				switch (mesh->mesh_type)
				{
				case MESH_TYPE::CUBE:
				case MESH_TYPE::SPHERE:
					rootelement->InsertEndChild(WriteMaterial(mFile, mesh->materials[0]));
					break;
				case MESH_TYPE::IMPORTED:
					std::string path = global::ActiveSceneDir + "Assets\\" + node->Parent->Name + std::to_string(node->Parent->Id) + ".cmesh";

					for (auto& material : mesh->materials)
					{
						rootelement->InsertEndChild(WriteMaterial(mFile, material));
					}

					tinyxml2::XMLElement* meshdataelement = mFile->NewElement("MeshData");
					rootelement->InsertEndChild(meshdataelement);

					tinyxml2::XMLElement* pathelement = mFile->NewElement("Path");
					pathelement->SetText(path.c_str());
					meshdataelement->InsertEndChild(pathelement);

					break;
				}

				break;
			}
		case NODE_DATA_TYPE::LIGHT:
			{
				Light* light = static_cast<Light*>(node);
				addelement("InternalType", static_cast<int>(light->Type));

				rootelement->InsertEndChild(WriteBinaryData(mFile, "Data", light->Data));

				break;
			}
		}

		auto vec3attribute = [&](const char* name, const glm::vec3& data) {
			tinyxml2::XMLElement* element = mFile->NewElement(name);
			element->SetAttribute("x", data.x);
			element->SetAttribute("y", data.y);
			element->SetAttribute("z", data.z);
			return element;
		};

		//Write Node Transform
		tinyxml2::XMLElement* transform = mFile->NewElement("Transform");
		transform->InsertEndChild(vec3attribute("Position", node->NodeTransform->Position));
		transform->InsertEndChild(vec3attribute("Rotation", glm::degrees(node->NodeTransform->Rotation)));
		transform->InsertEndChild(vec3attribute("Scale", node->NodeTransform->Scale));
		rootelement->InsertEndChild(transform);

		//Write Children Nodes ID
		for (auto& child : node->Children)
		{
			addelement("Child", child->Id);
		}
	}

	void ReadBinaryData(const tinyxml2::XMLElement* element, std::vector<char>& data)
	{
		std::string decodeddata = base64_decode(element->GetText());

		data.resize(decodeddata.size());
		memcpy(data.data(), decodeddata.data(), decodeddata.size());
	}

	void ReadMaterial(const tinyxml2::XMLElement* element, Material* material)
	{
		material->Name = element->FirstChildElement("Name")->GetText();
		ReadBinaryData(element->FirstChildElement("Data"), material->Data);

		const tinyxml2::XMLElement* texturemapelement = element->FirstChildElement("TextureMap");
		
		while (texturemapelement)
		{
			int texturemaptype;
			texturemapelement->QueryIntAttribute("Type", &texturemaptype);
			TEXTURE_MAP_TYPE texture_map_type = static_cast<TEXTURE_MAP_TYPE>(texturemaptype);

			const char* mapfilepath = texturemapelement->GetText();

			auto& it = material->TextureMaps.find(texture_map_type);
			if (it == material->TextureMaps.end())
			{
				material->TextureMaps.insert({ texture_map_type, new TextureMap() });
				it = material->TextureMaps.find(texture_map_type);
			}

			if (mapfilepath)
			{
				it->second->filepath = mapfilepath;
				it->second->texture = new Texture2D(LoadTexture2D(mapfilepath));
			}

			texturemapelement = texturemapelement->NextSiblingElement("TextureMap");
		}
	}

	Node* XMLFile::ReadNode(uint32_t id, std::vector<uint32_t>& childrenid)
	{
		const tinyxml2::XMLElement* rootelement = mFile->FirstChildElement(("Node" + std::to_string(id)).c_str());
		int type;
		rootelement->QueryIntAttribute("Type", &type);

		NODE_DATA_TYPE node_data_type = static_cast<NODE_DATA_TYPE>(type);

		const char* nodename = rootelement->FirstChildElement("Name")->GetText();

		int internaltype;
		const tinyxml2::XMLElement* internaltypeelement = rootelement->FirstChildElement("InternalType");
		if (internaltypeelement)
		{
			internaltypeelement->QueryIntText(&internaltype);
		}

		Node* node = {};

		switch (node_data_type)
		{
		case NODE_DATA_TYPE::MESH:
			{
				MESH_TYPE mesh_type = static_cast<MESH_TYPE>(internaltype);

				Mesh* mesh = {};

				switch (mesh_type)
				{
				case MESH_TYPE::CUBE: 
					mesh = Cube(nodename); 
					ReadMaterial(rootelement->FirstChildElement("Material"), mesh->materials[0]); 
					break;
				case MESH_TYPE::SPHERE: 
					mesh = Sphere(nodename);
					ReadMaterial(rootelement->FirstChildElement("Material"), mesh->materials[0]);
					break;
				case MESH_TYPE::IMPORTED:
					mesh = new Mesh();
					mesh->Name = nodename;
					mesh->mesh_type = mesh_type;
					mesh->boundingbox = CalculateBoundingBox(nullptr, 0, 0);

					const tinyxml2::XMLElement* materialelement = rootelement->FirstChildElement("Material");
					
					while (materialelement)
					{
						mesh->materials.push_back(new Material());
						ReadMaterial(materialelement, mesh->materials[mesh->materials.size() - 1]);
						materialelement = materialelement->NextSiblingElement("Material");
					}

					const tinyxml2::XMLElement* meshdataelement = rootelement->FirstChildElement("MeshData");

					const char* meshpath = meshdataelement->FirstChildElement("Path")->GetText();

					//Open cmesh File
					XMLFile* cmesh = new XMLFile();
					if (cmesh->Load(meshpath))
					{
						std::string meshname = mesh->Name;
						meshname.erase(std::remove(meshname.begin(), meshname.end(), ' '), meshname.end());

						std::vector<char> buffer;

						std::vector<float> vertices;
						ReadBinaryData(cmesh->GetFile()->FirstChildElement(meshname.c_str())->FirstChildElement("Vertices"), buffer);

						vertices.resize(buffer.size() / sizeof(float));
						memcpy(vertices.data(), buffer.data(), buffer.size());

						buffer.clear();

						std::vector<uint32_t> indices;
						ReadBinaryData(cmesh->GetFile()->FirstChildElement(meshname.c_str())->FirstChildElement("Indices"), buffer);

						indices.resize(buffer.size() / sizeof(uint32_t));
						memcpy(indices.data(), buffer.data(), buffer.size());

						mesh->vertexarray = new VertexArray();
						mesh->vertexarray->VertexBuffer(vertices.data(), vertices.size() * sizeof(float), "332");
						mesh->vertexarray->IndexBuffer(indices.data(), (uint32_t)indices.size());

						mesh->boundingbox = CalculateBoundingBox(vertices.data(), (uint32_t)vertices.size(), 8);
					}

					delete cmesh;
					break;
				}

				node = mesh;
				break;
			}
		case NODE_DATA_TYPE::LIGHT:
			{
				LIGHT_TYPE light_type = static_cast<LIGHT_TYPE>(internaltype);

				Light* light = {};

				switch (light_type)
				{
				case LIGHT_TYPE::DIRECTIONAL:light = CreateDiretionalLight(nodename); break;
				case LIGHT_TYPE::POINT:light = CreatePointLight(nodename); break;
				}

				ReadBinaryData(rootelement->FirstChildElement("Data"), light->Data);

				node = light;
				break;
			}
		default:
			{
				node = new Node();
				node->Name = nodename;
				node->node_data_type = node_data_type;
			}
		}

		const tinyxml2::XMLElement* transform = rootelement->FirstChildElement("Transform");

		auto vec3attribute = [&transform](const char* name, glm::vec3& data) {
			const tinyxml2::XMLElement* element = transform->FirstChildElement(name);
			element->QueryFloatAttribute("x", &data.x);
			element->QueryFloatAttribute("y", &data.y);
			element->QueryFloatAttribute("z", &data.z);
		};

		vec3attribute("Position", node->NodeTransform->Position);
		glm::vec3 rotation;
		vec3attribute("Rotation", rotation);
		node->NodeTransform->Rotation = glm::radians(rotation);
		vec3attribute("Scale", node->NodeTransform->Scale);

		const tinyxml2::XMLElement* childelement = rootelement->FirstChildElement("Child");

		while (childelement)
		{
			uint32_t childid;
			childelement->QueryUnsignedText(&childid);
			childrenid.push_back(childid);

			childelement = childelement->NextSiblingElement("Child");
		}

		return node;
	}

	void XMLFile::Save(const std::string& dstFile)
	{
		mFile->SaveFile(dstFile.c_str());
	}

	bool XMLFile::Load(const std::string& srcFile)
	{
		auto result = mFile->LoadFile(srcFile.c_str());
		if (result != tinyxml2::XML_SUCCESS)
		{
			std::string msg = "Failed To Open " + srcFile;
			return Message<ERROR_MSG>(msg.c_str(), MESSAGE_ORIGIN::FILESYSTEM);
		}
		return true;
	}

}