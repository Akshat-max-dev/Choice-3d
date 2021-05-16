#include "SceneHierarchy.h"

#include "FontAwesome.h"
#include <imgui.h>
#include "FileDialog.h"

namespace choice
{
	SceneHierarchy::SceneHierarchy()
	{
	}
	SceneHierarchy::~SceneHierarchy()
	{
	}

	void SceneHierarchy::Execute(Scene* scene)
	{
		//Scene Hierarchy
		ImGui::Begin(ICON_FK_LIST_UL" Hierarchy");

		std::string icon = ICON_FK_PICTURE_O;
		if (ImGui::CollapsingHeader((icon + " " + scene->Name()).c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (uint32_t i = 0; i < scene->GetNodes().size(); i++)
			{
				IterateNode(scene->GetNodes()[i]); //Iterate Each Scene Node
			}
		}

		ShowAddingMenu(scene);

		ImGui::End();//Scene Hierarchy
	}

	void SceneHierarchy::IterateNode(Node* node)
	{
		if (node)
		{
			if (ImGui::TreeNodeEx(node->Name.c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick))
			{
				if (ImGui::IsItemClicked())
				{
					mSelectedNode = node;
				}

				if (ImGui::BeginPopupContextItem())
				{
					if (ImGui::MenuItem("Sphere"))
					{
						node->Children.push_back(Sphere());
						node->Children[node->Children.size() - 1]->Parent = node;
					}
					ImGui::EndPopup();
				}

				for (auto& child : node->Children)
				{
					IterateNode(child);
				}
				ImGui::TreePop();
			}
		}
	}

	void ShowAddingMenu(Scene* scene)
	{
		if (ImGui::BeginPopupContextWindow(0, 1, false))
		{
			if (ImGui::BeginMenu("Add Mesh"))
			{
				//Add Cube
				if (ImGui::MenuItem("Cube"))
				{
					scene->AddNode(Cube());
				}

				//Add Sphere
				if (ImGui::MenuItem("Sphere"))
				{
					scene->AddNode(Sphere());
				}

				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("Change Skybox"))
			{
				std::string hdri = FileDialog::OpenFile("HDRI (*.hdr,*.exr)\0*.hdr\0*.exr\0");
				if (!hdri.empty())
				{
					std::string hdriname = hdri.substr(hdri.find_last_of('\\') + 1, hdri.size());

					std::string dsthdri = scene->Directory() + "\\" +
						scene->Name() + "\\Assets\\" + hdriname;

					ghc::filesystem::copy_file(hdri, dsthdri);
					ghc::filesystem::remove(scene->GetSkybox()->GetFilepath());
					delete scene->GetSkybox();

					scene->SetSkybox(new Skybox(dsthdri));
				}
			}

			if (ImGui::BeginMenu("Add Light"))
			{
				if (ImGui::MenuItem("Directional Light"))
				{
					DirectionalLight* directionallight = new DirectionalLight();
					directionallight->Name = "Directional Light";
					directionallight->Type = LIGHT_TYPE::DIRECTIONAL;
					directionallight->node_data_type = NODE_DATA_TYPE::LIGHT;
					scene->AddNode(directionallight);
				}
				if (ImGui::MenuItem("Point Light"))
				{
					PointLight* pointlight = new PointLight();
					pointlight->Name = "Point Light";
					pointlight->Type = LIGHT_TYPE::POINT;
					pointlight->node_data_type = NODE_DATA_TYPE::LIGHT;
					scene->AddNode(pointlight);
				}
				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}
	}

}