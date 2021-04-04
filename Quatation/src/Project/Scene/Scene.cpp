#include "Scene.h"

#include <glad/glad.h>

namespace choice
{
	std::string ReadPropertyType(std::ifstream& containedscene)
	{
		uint32_t proptypesize;
		containedscene.read((char*)&proptypesize, sizeof(proptypesize));
		std::string proptype;
		proptype.resize(proptypesize);
		containedscene.read((char*)proptype.data(), proptypesize);
		return proptype;
	}

	Scene::Scene(const std::string& name, const std::string& directory)
		:mName(name), mDirectory(directory)
	{
		_mkdir((mDirectory + "\\" + mName).c_str());
		_mkdir((mDirectory + "\\" + mName + "\\" + "Assets").c_str());
	}

	Scene::Scene(const std::string& cscene)
	{
		std::string temp = cscene.substr(cscene.find_last_of('\\') + 1, cscene.size());
		mName = temp.substr(0, temp.find_last_of('.'));
		temp = cscene.substr(0, cscene.find_last_of('\\'));
		mDirectory = temp.substr(0, temp.size() - mName.size() - 1);

		std::ifstream containedscene(cscene, std::ios::in | std::ios::binary);
		if (containedscene.bad() && !containedscene.is_open())
		{
			std::cout << "Failed To Load Scene" << std::endl;
			return;
		}

		uint32_t sceneobjectssize;
		containedscene.read((char*)&sceneobjectssize, sizeof(sceneobjectssize));
		mSceneObjects.resize(sceneobjectssize);
		for (auto& object : mSceneObjects)
		{
			object = new SceneObject();
			std::string proptype = ReadPropertyType(containedscene);

			if (proptype == "Model")
			{
				uint32_t srcFilesize;
				containedscene.read((char*)&srcFilesize, sizeof(srcFilesize));
				std::string srcFile;
				srcFile.resize(srcFilesize);
				containedscene.read((char*)srcFile.data(), srcFilesize);

				object->AddProperty<Model>(LoadModel(srcFile));
			}
		}
		containedscene.close();
	}

	Scene::~Scene()
	{
		for (auto& object : mSceneObjects)
		{
			if (object) { delete object; }
		}
	}

	void Scene::Save()
	{
		std::ofstream cscene(mDirectory + "\\" + mName + "\\" + mName + ".cscene", std::ios::out | std::ios::binary);
		if (cscene.fail())
		{
			std::cout << "Cannot Save Scene" << std::endl;
			return;
		}

		uint32_t sceneobjectssize = (uint32_t)mSceneObjects.size();
		cscene.write((char*)&sceneobjectssize, sizeof(sceneobjectssize));
		for (auto& object : mSceneObjects)
		{
			if (object)
			{
				std::string proptype;
				uint32_t proptypesize = 0;

				auto modelprop = object->GetProperty<Model>();
				if (modelprop)
				{
					proptype = "Model";
					proptypesize = (uint32_t)proptype.size();
					cscene.write((char*)&proptypesize, sizeof(proptypesize));
					cscene.write((char*)proptype.data(), proptypesize);

					std::string srcFile = mDirectory + "\\" + mName + "\\" + "Assets\\" + modelprop->Name + ".cmodel";
					uint32_t srcFileSize = (uint32_t)srcFile.size();
					cscene.write((char*)&srcFileSize, sizeof(srcFileSize));
					cscene.write(srcFile.data(), srcFileSize);
				}
			}
		}

		cscene.close();
	}

}