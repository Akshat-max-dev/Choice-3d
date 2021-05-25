#include "Scene.h"

#include <glad/glad.h>
#include "BinaryHelper.h"

#include "XML/XMLFile.h"

namespace choice
{
	namespace xml = tinyxml2;

	Scene::Scene(const std::string& name, const std::string& directory)
		:mName(name), mDirectory(directory)
	{
		NodeCounter = 0;

		ghc::filesystem::create_directory(mDirectory + "\\" + mName);
		ghc::filesystem::create_directory(mDirectory + "\\" + mName + "\\" + "Assets");

		ghc::filesystem::copy("Choice/assets/hdri_skybox/Road_to_MonumentValley_Ref.hdr", 
			mDirectory + "\\" + mName + "\\" + "Assets\\Road_to_MonumentValley_Ref.hdr");

		std::string dstHDRI = mDirectory + "\\" + mName + "\\" + "Assets\\Road_to_MonumentValley_Ref.hdr";
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
		cscene->Load(srcFile);

		//Create Skybox
		mSkybox = new Skybox(cscene->AttributeElement<const char*>("Skybox"));

		const uint32_t nodecount = cscene->AttributeElement<uint32_t>("NodeCount");

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

		NodeCounter = nodecount;

		for (auto& node : nodes)
		{
			if (!node->Parent)
				mNodes.push_back(node);
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

	void Scene::Save()
	{
		XMLFile* cscene = new XMLFile();

		cscene->AttributeElement<const char*>("Skybox", mSkybox->GetFilepath().c_str());
		for (auto& node : mNodes)
		{
			IterateNodes(node, [&cscene](Node* node) {
				cscene->WriteNode(node);
			});
		}

		cscene->AttributeElement<uint32_t>("NodeCount", NodeCounter);

		cscene->Save(mDirectory + "\\" + mName + "\\" + mName + ".cscene");

		delete cscene;
	}

	void Scene::Clean()
	{
		/*//TODO : Fix For CUBE And SPHERE
		std::string cscenefile = mDirectory + "\\" + mName + "\\" + mName + ".cscene";
		std::ifstream cscene(cscenefile, std::ios::in | std::ios::binary);
		
		uint32_t sceneobjectssize;
		cscene.read((char*)&sceneobjectssize, sizeof(sceneobjectssize));

		for (uint32_t i = sceneobjectssize; i < mSceneObjects.size(); i++)
		{
			Skybox* skybox = mSceneObjects[i]->GetProperty<Skybox>();
			if (skybox)
			{
				ghc::filesystem::remove(skybox->GetFilepath());
			}

			Drawable* drawable = mSceneObjects[i]->GetProperty<Drawable>();
			if (drawable)
			{
				if (drawable->GetDrawableType() == DrawableType::MODEL)
				{
					ghc::filesystem::remove(mDirectory + "\\" + mName + "\\" + "Assets\\" + drawable->GetName() + ".cmodel");
					ghc::filesystem::remove(mDirectory + "\\" + mName + "\\" + "Assets\\" + drawable->GetName() + ".cmaterial");
				}
				for (auto& material : drawable->GetMaterials())
				{
					if (material->DiffuseMap.second.second)
					{
						if (!material->DiffuseMap.second.second->Source.empty())
						{
							ghc::filesystem::remove(material->DiffuseMap.second.second->Source);
						}
					}
					if (material->NormalMap.second.second)
					{
						if (!material->NormalMap.second.second->Source.empty())
						{
							ghc::filesystem::remove(material->NormalMap.second.second->Source);
						}
					}
				}
			}
		}

		cscene.close();*/
	}
}