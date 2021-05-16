#include "ProjectExplorer.h"

#include "FontAwesome.h"

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
			ImGui::Begin(ICON_FK_FOLDER_OPEN_O" Explorer");

			std::string icon = ICON_FK_FOLDER_OPEN;
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
				std::string foldericon = ICON_FK_FOLDER_O;
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
					std::string fileicon = ICON_FK_FILE_O;
					if (ImGui::TreeNodeEx((fileicon + " " + f.filename().string()).c_str(), ImGuiTreeNodeFlags_Leaf))
					{
						ImGui::TreePop();
					}
				}
			}
		}
	}

}