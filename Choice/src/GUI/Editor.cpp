#include "Editor.h"

#include <imgui.h>
#include <ImGuiFileDialog.h>

#include <glad/glad.h>
#include <dirent/dirent.h>
#include "Input.h"

#include "Choice.h"

namespace choice
{
	Editor::Editor(uint32_t w, uint32_t h)
	{
		mCamera = std::make_unique<EditorCamera>((float)w / (float)h);

		mShader = std::make_unique<Shader>("Choice/assets/shaders/Test.glsl");
	}

	Editor::~Editor()
	{
		
	}

	void Editor::Execute()
	{
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
		
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New Project", "Ctrl + N"))
				{
					mModalPurpose = ModalPurpose::NEWPROJECT; mShowModal = true;
				}

				if (ImGui::MenuItem("Save Project", "Ctrl + S"))
				{
					mActiveProject->Save();
				}

				if (ImGui::MenuItem("Open Project", "Ctrl + O"))
				{
					ImGuiFileDialog::Instance()->SetExtentionInfos(".cproj", { 1.0f, 0.0f, 1.0f, 1.0f });
					ImGuiFileDialog::Instance()->OpenModal("OpenProject", "Open Project", ".cproj", "");
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Blender"))
				{
#ifdef EXE
					ImGuiFileDialog::Instance()->SetExtentionInfos(".exe", { 0.5f, 0.1f, 0.6f, 1.0f });
					ImGuiFileDialog::Instance()->OpenModal("LinkBlender", "Link Blender", ".exe", "");
#endif
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		auto* viewport = ImGui::GetMainViewport();
		static float IFDModalWidth = viewport->Size.x / 2;
		static float IFDModalHeight = viewport->Size.y / 2 + 100.0f;

		if (mShowModal)
		{
			switch (mModalPurpose)
			{
			case ModalPurpose::NEWPROJECT:
				ImGui::OpenPopup("New Project");
				ImGui::SetNextWindowSize({ 349.0f, 128.0f }, ImGuiCond_Appearing);
				if (ImGui::BeginPopupModal("New Project", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
				{
					ImGui::Text("Name :");
					ImGui::SameLine();
					static char namebuf[32] = "";
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
					ImGui::InputText("##Name", namebuf, 32);

					static char dirbuf[256] = "";
					if (ImGui::BeginTable("##Directory", 3, ImGuiTableFlags_SizingFixedFit))
					{
						for (int row = 0; row < 1; row++)
						{
							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("Directory :");
							ImGui::TableSetColumnIndex(1);
							ImGui::SetNextItemWidth(225.0f);
							ImGui::InputText("##Directory", dirbuf, 256);
							ImGui::TableSetColumnIndex(2);
							if (ImGui::Button("..."))
							{
								ImGuiFileDialog::Instance()->OpenModal("ChooseDirectory", "Choose Directory", nullptr, "");
							}
						}
						ImGui::EndTable();
					}

					if (ImGuiFileDialog::Instance()->Display("ChooseDirectory", ImGuiWindowFlags_NoCollapse, { IFDModalWidth, IFDModalHeight }))
					{
						if (ImGuiFileDialog::Instance()->IsOk())
						{
							std::string dir = ImGuiFileDialog::Instance()->GetCurrentPath();
							for (uint32_t i = 0; i < dir.size(); i++)
							{
								dirbuf[i] = dir[i];
							}
						}
						ImGuiFileDialog::Instance()->Close();
					}

					ImGui::Separator();

					if (ImGui::Button("Create"))
					{
						if (strlen(namebuf) == 0) { std::cout << "Project Name Cant Be Empty" << std::endl; return; }
						if (strlen(dirbuf) == 0) { std::cout << "No Directory Selected" << std::endl; return; }

						if (mActiveProject) { mActiveProject.reset(); }
						mActiveProject = std::make_unique<Project>(namebuf, dirbuf);
						memset(namebuf, 0, sizeof(namebuf));
						memset(dirbuf, 0, sizeof(dirbuf));

						Choice::Instance()->GetWindow()->UpdateTitle(("Choice | " + mActiveProject->Name() + " |").c_str());

						mShowModal = false;
						mModalPurpose = ModalPurpose::NONE;
					}
					ImGui::SameLine();
					if (ImGui::Button("Cancel"))
					{
						memset(namebuf, 0, sizeof(namebuf));
						memset(dirbuf, 0, sizeof(dirbuf));
						mShowModal = false;
						mModalPurpose = ModalPurpose::NONE;
					}

					ImGui::EndPopup();
				}
				break;
			}
		}

		if (ImGuiFileDialog::Instance()->Display("OpenProject", ImGuiWindowFlags_NoCollapse, { IFDModalWidth, IFDModalHeight }))
		{
			if (ImGuiFileDialog::Instance()->IsOk())
			{
				std::string cproj = ImGuiFileDialog::Instance()->GetFilePathName();
				if (mActiveProject) { mActiveProject.reset(); }
				mActiveProject = std::make_unique<Project>(cproj);
				Choice::Instance()->GetWindow()->UpdateTitle(("Choice | " + mActiveProject->Name() + " |").c_str());
			}
			ImGuiFileDialog::Instance()->Close();
		}

		if (ImGuiFileDialog::Instance()->Display("LinkBlender", ImGuiWindowFlags_NoCollapse, { IFDModalWidth, IFDModalHeight }))
		{
			if (ImGuiFileDialog::Instance()->IsOk())
			{
				std::string path = ImGuiFileDialog::Instance()->GetFilePathName();
				std::string name = ImGuiFileDialog::Instance()->GetCurrentFileName();
#ifdef EXE
				if (name != "blender.exe")
				{
					std::cout << "The Selected File Is Not blender.exe" << std::endl;
					ImGuiFileDialog::Instance()->Close();
					return;
				}
				char c = '"';
				std::ofstream writebat("gltf.bat", std::ios::out);

				if (writebat.fail() && writebat.bad())
				{
					std::cout << "Couldn't write Batch File" << std::endl;
					ImGuiFileDialog::Instance()->Close();
					return;
				}

				//Absolute Path To gltf.py
				std::string abspath = std::filesystem::absolute("Choice/assets/scripts/gltf.py").string();

				writebat << "call " << c + path + c << " --background --python " << c + abspath + c << std::endl;
				writebat << "cls";

				writebat.close();
#endif
			}
			ImGuiFileDialog::Instance()->Close();
		}

		if (ImGuiFileDialog::Instance()->Display("ImportModel", ImGuiWindowFlags_NoCollapse, { IFDModalWidth, IFDModalHeight }))
		{
			if (ImGuiFileDialog::Instance()->IsOk())
			{
				std::string modelfilepath = ImGuiFileDialog::Instance()->GetFilePathName();
				
				std::ifstream checkforbat("gltf.bat", std::ios::in);
				if (checkforbat.fail())
				{
					std::cout << "Blender Not Linked" << std::endl;
					ImGuiFileDialog::Instance()->Close();
					return;
				}
				checkforbat.close();

				std::ofstream temporary("Temporary", std::ios::out);
				if (temporary.bad() || temporary.fail())
				{
					std::cout << "Cannot Write Temporary File" << std::endl;
					ImGuiFileDialog::Instance()->Close();
					return;
				}
				temporary << modelfilepath;
				temporary.close();
				
				std::system("gltf.bat");
				std::remove("Temporary");
				
				if (mModel)
				{
					delete mModel;
					mModel = LoadModel(modelfilepath.substr(0, modelfilepath.find_last_of('.')) + ".glb");
					std::remove((modelfilepath.substr(0, modelfilepath.find_last_of('.')) + ".glb").c_str());
				}
			}
			ImGuiFileDialog::Instance()->Close();
		}
	}

	void Editor::Update()
	{
		mCamera->Update();
	}

}