#pragma once
#include "cpch.h"

#include "SceneObject/SceneObject.h"

namespace choice
{
	class Scene
	{
	public:
		Scene(const std::string& name, const std::string& directory);
		Scene(const std::string& cscene);
		~Scene();

		void AddObject(SceneObject* sceneobject) { mSceneObjects.push_back(sceneobject); }
		void DeleteObject(uint32_t index) { delete mSceneObjects[index]; mSceneObjects[index] = nullptr; }

		std::vector<SceneObject*>& GetSceneObjects() { return mSceneObjects; }

		void Save();
		void Clean();

		std::string& Name() { return mName; }
		const std::string& Directory()const { return mDirectory; }
	private:
		std::string mName;
		std::string mDirectory;
		std::vector<SceneObject*> mSceneObjects;
	};
}