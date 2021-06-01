#include "ProjectExplorer.h"

#include "IconsFontAwesome5.h"

#include <imgui.h>

namespace choice
{
	ProjectExplorer::ProjectExplorer()
	{
	}
	ProjectExplorer::~ProjectExplorer()
	{
	}

	void ProjectExplorer::Execute(Project* project)
	{
		if (project)
		{
			ImGui::Begin(ICON_FA_FOLDER" Explorer");

			std::string icon = ICON_FA_FOLDER_OPEN;
			if (ImGui::TreeNodeEx((icon + " " + project->Name()).c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Leaf))
			{
				IterateDirectory(project->Directory() + "\\" + project->Name()); //Iterate Project Directory
				ImGui::TreePop();
			}

			ImGui::End();
		}
	}

	void ProjectExplorer::IterateDirectory(const std::string& directory)
	{
		for (auto& it : ghc::filesystem::directory_iterator(directory))
		{
			auto& f = it.path();
			if (ghc::filesystem::is_directory(f))
			{
				std::string foldericon = ICON_FA_FOLDER;
				if (ImGui::TreeNodeEx((foldericon + " " + f.filename().string()).c_str(), ImGuiTreeNodeFlags_OpenOnArrow))
				{
					IterateDirectory(f.string());
					ImGui::TreePop();
				}
			}
			else
			{
				std::string ext = f.filename().extension().string();
				if (!(ext == ".cproj" || ext == ".cscene"))
				{
					std::string fileicon = ICON_FA_FILE;
					if (ImGui::TreeNodeEx((fileicon + " " + f.filename().string()).c_str(), ImGuiTreeNodeFlags_Leaf))
					{
						ImGui::TreePop();
					}
				}
			}
		}
	}

}