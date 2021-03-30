#include "Scene.h"

#include <glad/glad.h>

namespace choice
{
	Scene::Scene(const std::string& name, const std::string& directory)
		:mName(name), mDirectory(directory)
	{
		_mkdir((mDirectory + "\\" + mName).c_str());
		_mkdir((mDirectory + "\\" + mName + "\\" + "Assets").c_str());

		std::ofstream cscenedata(mDirectory + "\\" + mName + "\\" + mName + ".cscenedata", std::ios::out | std::ios::binary);
		cscenedata.close();
	}

	Scene::Scene(const std::string& cscene)
	{
		std::string temp = cscene.substr(cscene.find_last_of('\\') + 1, cscene.size());
		mName = temp.substr(0, temp.find_last_of('.'));
		temp = cscene.substr(0, cscene.find_last_of('\\'));
		mDirectory = temp.substr(0, temp.size() - mName.size() - 1);
	}

	Scene::~Scene()
	{
		for (auto& model : mModels)
		{
			delete model.second;
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
		cscene.close();
	}

}