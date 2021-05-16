#pragma once
#include "cpch.h"

#include "Project/Project.h"

namespace choice
{
	class ProjectExplorer
	{
	public: 
		ProjectExplorer();
		~ProjectExplorer();

		void Execute(Project* project);
	private:
		void IterateDirectory(const std::string& directory);
	};
}