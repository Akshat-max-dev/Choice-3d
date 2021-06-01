#include "SceneHierarchy.h"

#include "IconsFontAwesome5.h"
#include <imgui.h>
#include "FileDialog.h"
#include "Project/Scene/Nodes/gltfImport.h"
#include "Error.h"
#include "Input.h"

namespace choice
{
	SceneHierarchy::SceneHierarchy()
	{
	}
	SceneHierarchy::~SceneHierarchy()
	{
	}

	static void AddingMenuItems(Scene* scene)
	{
		if (ImGui::MenuItem("Empty"))
		{
			Node* node = new Node();
			node->node_data_type = NODE_DATA_TYPE::NONE;
			node->Name = "Empty";
			scene->AddNode(node);
		}

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

		if (ImGui::MenuItem("Import Model"))
		{
			std::string filepath = FileDialog::OpenFile("All Files");
			if (!filepath.empty())
			{
				if (!ghc::filesystem::exists("gltf.bat"))
				{
					Message<ERROR_MSG>("Blender Not Linked", MESSAGE_ORIGIN::EDITOR);
				}

				std::string ext = ghc::filesystem::path(filepath).extension().string();

				if (!(ext == ".glb" || ext == ".gltf"))
				{
					std::ofstream temporary("Temporary", std::ios::out);
					temporary << filepath;
					temporary.close();

					std::system("gltf.bat");
					ghc::filesystem::remove("Temporary");
				}

				Node* root = new Node();
				root->Id = ++global::NodeCounter;

				if (ImportGLTF(filepath.substr(0, filepath.find_last_of('.')) + ".glb", root))
				{
					scene->AddNode(root);
					ghc::filesystem::remove(filepath.substr(0, filepath.find_last_of('.')) + ".glb");
				}
				else
				{
					delete root;
				}
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
	}

	void SceneHierarchy::Execute(Scene* scene)
	{
		//Scene Hierarchy
		ImGui::Begin(ICON_FA_LIST_UL" Hierarchy");

		if (ImGui::IsWindowFocused() || ImGui::IsWindowHovered())
		{
			Choice::Instance()->GetEditor()->GetCamera()->AcceptInput(false);
		}

		//Add Button
		ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
		bool ispressed = ImGui::Button(ICON_FA_PLUS);
		ImGui::PopStyleColor();

		if (ispressed)
			ImGui::OpenPopup("AddingMenu");

		if (ImGui::BeginPopup("AddingMenu"))
		{
			AddingMenuItems(scene);
			ImGui::EndPopup();
		}

		ImGui::SameLine();

		//Search Bar
		static char searchbuf[512] = "";
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
		ImGui::InputTextWithHint("##SearchHierarchy", ICON_FA_SEARCH " Search Hierarchy",
			searchbuf, 512, ImGuiInputTextFlags_AutoSelectAll);

		ImGui::Separator();

		std::string icon = ICON_FA_GLOBE;

		ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
		ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
		ImGui::PushStyleColor(ImGuiCol_Text, { 0.3f, 0.8f, 0.2f, 1.0f });

		bool isopen = ImGui::CollapsingHeader((icon + " " + scene->Name()).c_str(), ImGuiTreeNodeFlags_OpenOnArrow |
			ImGuiTreeNodeFlags_DefaultOpen);

		ImGui::PopStyleColor(4);

		if (isopen)
		{
			if (strlen(searchbuf) != 0)
			{
				auto func = [&](Node* node) {
					if (strstr(node->Name.c_str(), searchbuf))
					{
						if (mSelectedNode == node)
							ImGui::PushStyleColor(ImGuiCol_Text, { 0.2f, 0.6f, 0.8f, 1.0f });;

						if (ImGui::TreeNodeEx(node->Name.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth))
						{
							if (mSelectedNode == node)
								ImGui::PopStyleColor();

							if (ImGui::IsItemClicked())
							{
								mSelectedNode = node;
							}
							ImGui::TreePop();
						}
					}
				};

				for (auto& node : scene->GetNodes())
				{
					IterateNodes(node, func);
				}
			}
			else
			{
				if (ImGui::BeginTable("##Hierarchy", 1, ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
				{
					ImGui::TableSetupColumn("##Column1", ImGuiTableColumnFlags_NoHide);

					for (auto& node : scene->GetNodes())
					{
						IterateNode(node); //Iterate Each Scene Node
					}

					ShowAddingMenu(scene);

					ImGui::EndTable();
				}
			}
		}

		ImGui::End();//Scene Hierarchy
	}

	void SceneHierarchy::IterateNode(Node*& node)
	{
		if (node)
		{
			ImGui::PushID(node->Id);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			static uint32_t nodeIdToRename = 0;
			static bool showTextInput = false;

			if (showTextInput && (node->Id == nodeIdToRename))
			{
				//Show Text Input If User Clicks Rename
				static char buf[512] = " ";
				
				static bool	highlightBorder; //Set This True If The User Enters An Empty Name

				if (strlen(buf) == 0)
					highlightBorder = true;
				else
					highlightBorder = false;
				
				if (!highlightBorder)
					memcpy(buf, node->Name.data(), node->Name.size());

				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());

				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 1.2f);
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
				ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
				ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);

				if (highlightBorder)
				{
					ImGui::PushStyleColor(ImGuiCol_Border, { 1.0f, 0.0f, 0.0f, 1.0f });
				}

				bool isenterpressed = ImGui::InputText("##Rename", buf, 512, ImGuiInputTextFlags_AutoSelectAll |
					ImGuiInputTextFlags_EnterReturnsTrue);

				ImGui::PopStyleVar(2);
				ImGui::PopStyleColor(highlightBorder ? 4 : 3);

				if (isenterpressed)
				{
					if (!highlightBorder)
					{
						node->Name.resize(strlen(buf));
						memcpy(node->Name.data(), buf, strlen(buf));
						memset(buf, 0, sizeof(buf));
						buf[0] = ' ';
						showTextInput = false;
						nodeIdToRename = 0;
					}
				}
				else if (Input::IsKeyPressed(Key::ESCAPE))
				{
					memset(buf, 0, sizeof(buf));
					buf[0] = ' ';
					showTextInput = false;
					highlightBorder = false;
					nodeIdToRename = 0;
				}
			}
			else
			{
				std::string icon = ICON_FA_CUBE;
				if (node->node_data_type == NODE_DATA_TYPE::LIGHT) { icon = ICON_FA_LIGHTBULB; }

				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanFullWidth;
				if (!node->Children.size())
				{
					flags |= ImGuiTreeNodeFlags_Leaf;
				}

				if (mSelectedNode == node)
					ImGui::PushStyleColor(ImGuiCol_Text, { 0.2f, 0.6f, 0.8f, 1.0f });

				bool isopen = ImGui::TreeNodeEx((icon + " " + node->Name).c_str(), flags);

				if (mSelectedNode == node)
					ImGui::PopStyleColor();

				if (mSelectedNode == node && Input::IsKeyPressed(Key::KDELETE))
				{
					delete node;
					mSelectedNode = nullptr;
					node = nullptr;
					isopen = false; //Make False So As Node Tree Code Is Not Executed
					ImGui::TreePop(); //Call This As A Tree Is Open Above
				}

				if (isopen)
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
							}

							if (ImGui::MenuItem("Sphere"))
							{
								node->Children.push_back(Sphere());
								node->Children[node->Children.size() - 1]->Parent = node;
							}
							ImGui::EndMenu();
						}

						if (ImGui::MenuItem("Delete"))
						{
							delete node;
							if (mSelectedNode == node)
								mSelectedNode = nullptr;
							node = nullptr;
						}

						if (ImGui::MenuItem("Rename"))
						{
							showTextInput = true;
							nodeIdToRename = node->Id;
						}

						ImGui::EndPopup();
					}

					if (node)
					{
						for (auto& child : node->Children)
						{
							IterateNode(child);
						}
					}
					ImGui::TreePop();
				}
			}

			ImGui::PopID();
		}
	}

	void ShowAddingMenu(Scene* scene)
	{
		if (ImGui::BeginPopupContextWindow(0, 1, false))
		{
			AddingMenuItems(scene);

			if (ImGui::MenuItem("Change Skybox"))
			{
				std::string hdri = FileDialog::OpenFile("HDRI (*.hdr,*.exr)\0*.hdr\0*.exr\0");
				if (!hdri.empty())
				{
					std::string hdriname = hdri.substr(hdri.find_last_of('\\') + 1, hdri.size());

					std::string dsthdri = scene->Directory() + "\\" +
						scene->Name() + "\\Assets\\" + hdriname;

					if (!ghc::filesystem::exists(dsthdri))
					{
						ghc::filesystem::copy_file(hdri, dsthdri);
					}

					ghc::filesystem::remove(scene->GetSkybox()->GetFilepath());
					delete scene->GetSkybox();

					scene->SetSkybox(new Skybox(dsthdri));
				}
			}

			ImGui::EndPopup();
		}
	}

}