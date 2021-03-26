#include "Scene.h"

namespace choice
{
	Scene::Scene(const std::string& name, const std::string& directory)
		:mName(name), mDirectory(directory)
	{
		_mkdir((mDirectory + "\\" + mName).c_str());
	}

	Scene::Scene(const std::string& scenedir)
	{

	}

	Scene::~Scene()
	{

	}

	void Scene::Save()
	{

	}

}