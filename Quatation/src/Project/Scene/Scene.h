#pragma once
#include "cpch.h"

namespace choice
{
	class Scene
	{
	public:
		Scene(const std::string& name, const std::string& directory);
		Scene(const std::string& scenedir);
		~Scene();

		void Save();

		std::string& Name() { return mName; }
	private:
		std::string mName;
		std::string mDirectory;
	};
}