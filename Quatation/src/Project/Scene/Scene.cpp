#include "Scene.h"

#include <glad/glad.h>

#include "XML/XMLFile.h"

namespace choice
{
	namespace xml = tinyxml2;

	Scene::Scene(const std::string& name, const std::string& directory)
		:mName(name), mDirectory(directory)
	{
		ghc::filesystem::create_directory(mDirectory + "\\" + mName);
		ghc::filesystem::create_directory(mDirectory + "\\" + mName + "\\" + "Assets");

		std::string dstHDRI = mDirectory + "\\" + mName + "\\" + "Assets\\Road_to_MonumentValley_Ref.hdr";

		ghc::filesystem::copy("Choice/assets/hdri_skybox/Road_to_MonumentValley_Ref.hdr", dstHDRI);
		
		mSkybox = new Skybox(dstHDRI);

		AddNode(CreateDiretionalLight());

		mBoundingBox = CalculateBoundingBox(nullptr, 0, 0);
	}

	Scene::Scene(const std::string& srcFile)
	{
		std::string temp = srcFile.substr(srcFile.find_last_of('\\') + 1, srcFile.size());
		mName = temp.substr(0, temp.find_last_of('.'));
		temp = srcFile.substr(0, srcFile.find_last_of('\\'));
		mDirectory = temp.substr(0, temp.size() - mName.size() - 1);

		mBoundingBox = CalculateBoundingBox(nullptr, 0, 0);

		XMLFile* cscene = new XMLFile();

		if (cscene->Load(srcFile))
		{
			//Create Skybox
			mSkybox = new Skybox(cscene->GetFile()->FirstChildElement("Skybox")->FirstChildElement("Path")->GetText());

			uint32_t nodecount;
			cscene->GetFile()->FirstChildElement("NodeCount")->FirstChildElement("Value")->QueryUnsignedText(&nodecount);

			std::vector<Node*> nodes;
			nodes.resize(nodecount);

			//Start From Last So As To Set Children Correctly
			for (uint32_t i = nodecount; i >= 1; i--)
			{
				uint32_t nodeIndex = i - 1;

				std::vector<uint32_t> childrenid;
				nodes[nodeIndex] = cscene->ReadNode(i, childrenid);
				if (nodes[nodeIndex])
				{
					nodes[nodeIndex]->Id = i;
					for (auto& id : childrenid)
					{
						uint32_t childnodeindex = id - 1;
						nodes[nodeIndex]->Children.push_back(nodes[childnodeindex]);
						nodes[nodeIndex]->Children[nodes[nodeIndex]->Children.size() - 1]->Parent = nodes[nodeIndex];
					}
				}
			}

			for (auto& node : nodes)
			{
				if (!node->Parent)
					mNodes.push_back(node);
			}

		}

		delete cscene;
	}

	Scene::~Scene()
	{
		for (auto& node : mNodes)
		{
			if (node)
			{
				delete node;
			}
		}

		delete mSkybox;
	}

	void Scene::AddNode(Node* node)
	{
		mNodes.push_back(node);
	}

	void Scene::Save()
	{
		XMLFile* cscene = new XMLFile();

		auto* skyboxelement = cscene->GetFile()->NewElement("Skybox");
		cscene->GetFile()->InsertEndChild(skyboxelement);

		auto* pathelement = cscene->GetFile()->NewElement("Path");
		pathelement->SetText(mSkybox->GetFilepath().c_str());
		skyboxelement->InsertEndChild(pathelement);

		for (auto& node : mNodes)
		{
			IterateNodes(node, [&cscene](Node* node) {
				cscene->WriteNode(node);
			});
		}

		auto* nodecountelement = cscene->GetFile()->NewElement("NodeCount");
		cscene->GetFile()->InsertEndChild(nodecountelement);

		auto* valuelement = cscene->GetFile()->NewElement("Value");
		valuelement->SetText(global::NodeCounter);
		nodecountelement->InsertEndChild(valuelement);

		cscene->Save(mDirectory + "\\" + mName + "\\" + mName + ".cscene");

		delete cscene;
	}
}