#pragma once
#include "cpch.h"

#include "Scene/Scene.h"

namespace choice
{
	class Project
	{
	public:
		Project(const std::string& name, const std::string& path);
		Project(const std::string& cproj);
		~Project();

		const std::string& Name()const { return mName; }
		const std::string& Directory()const { return mDirectory; }

		Scene*& ActiveScene() { return mActiveScene; }

		void Save();
	private:
		std::string mName;
		std::string mDirectory;
		Scene* mActiveScene;
	};
}