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

		std::string icon = ICON_FK_GLOBE;

		ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
		ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);

		if (ImGui::CollapsingHeader((icon + " " + scene->Name()).c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PopStyleColor(3);

			if (ImGui::BeginTable("##Hierarchy", 1, ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
			{
				ImGui::TableSetupColumn("##Column1", ImGuiTableColumnFlags_NoHide);

				for (uint32_t i = 0; i < scene->GetNodes().size(); i++)
				{
					IterateNode(scene->GetNodes()[i]); //Iterate Each Scene Node
				}

				ShowAddingMenu(scene);

				ImGui::EndTable();
			}
		}

		ImGui::End();//Scene Hierarchy
	}

	void SceneHierarchy::IterateNode(Node* node)
	{
		ImGui::PushID(node->Id);
		if (node)
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			std::string icon = ICON_FK_CUBE;
			if (node->node_data_type == NODE_DATA_TYPE::LIGHT) { icon = ICON_FK_LIGHTBULB_O; }

			ImGuiTreeNodeFlags flags = (mSelectedNode == node) ? ImGuiTreeNodeFlags_Selected : 0;
			flags |= ImGuiTreeNodeFlags_SpanFullWidth;

			if (!node->Children.size())
			{
				flags |= ImGuiTreeNodeFlags_Leaf;
			}

			if (ImGui::TreeNodeEx((icon + " " + node->Name).c_str(), flags))
			{
				if (ImGui::IsItemClicked())
				{
					mSelectedNode = node;
				}

				if (ImGui::BeginPopupContextItem())
				{
					if (ImGui::BeginMenu("Add Child"))
					{
						if (ImGui::MenuItem("Cube"))
						{
							node->Children.push_back(Cube());
							node->Children[node->Children.size() - 1]->Parent = node;

							NodeCounter++;
							node->Children[node->Children.size() - 1]->Id = NodeCounter;
						}

						if (ImGui::MenuItem("Sphere"))
						{
							node->Children.push_back(Sphere());
							node->Children[node->Children.size() - 1]->Parent = node;

							NodeCounter++;
							node->Children[node->Children.size() - 1]->Id = NodeCounter;
						}
						ImGui::EndMenu();
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

		ImGui::PopID();
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
					scene->AddNode(CreateDiretionalLight());
				}
				if (ImGui::MenuItem("Point Light"))
				{
					scene->AddNode(CreatePointLight());
				}
				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}
	}

}