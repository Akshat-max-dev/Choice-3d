#include "Editor.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuiFileDialog.h>
#include <ImGuizmo.h>

#include "IconsFontAwesome5.h"

#include "Input.h"
#include "Choice.h"
#include "Project/Scene/Nodes/Mesh.h"
#include "FileDialog.h"
#include "Error.h"

#include <glm/glm.hpp>

namespace choice
{
	Editor::Editor(uint32_t w, uint32_t h)
	{
		if (ghc::filesystem::exists("gltf.bat")) { mIsBlenderLinked = true; }

		if (ghc::filesystem::exists(".choiceeditorconfig"))
		{
			std::ifstream file(".choiceeditorconfig", std::ios::in | std::ios::binary);

			std::string cproj;
			file >> cproj;

			auto func = [&](glm::vec3& data) {
				file >> data.x >> data.y >> data.z;
			};

			glm::vec3 focus;
			func(focus);

			glm::vec3 offset;
			func(offset);

			glm::vec3 up;
			func(up);

			glm::vec3 right;
			func(right);

			mCamera = new EditorCamera((float)w / (float)h, focus, offset, up, right);

			if (!ghc::filesystem::exists(cproj)) { mActiveProject = {}; }
			else 
			{ 
				mActiveProject = new Project(cproj);
				Choice::Instance()->GetWindow()->UpdateTitle(("Choice | " + mActiveProject->Name() + " |").c_str());
			}

			file.close();
		}
		else
		{
			mCamera = new EditorCamera((float)w / (float)h);
			mActiveProject = {};
		}

		mSceneHierarchy = new SceneHierarchy();
		mNodeInspector = new NodeInspector();
		mProjectExplorer = new ProjectExplorer();
		mConsole = new Console();
	}

	Editor::~Editor()
	{
		if (mActiveProject)
		{
			//Write down the active project And Camera Data
			std::ofstream o(".choiceeditorconfig", std::ios::out | std::ios::binary);

			std::string cproj = global::ActiveProjectDir + global::ActiveProjectName + ".cproj";
			o << cproj << std::endl;

			auto func = [&](const glm::vec3 data) {
				o << data.x << " " << data.y << " " << data.z << std::endl;
			};

			func(mCamera->Focus());
			func(mCamera->Offset());
			func(mCamera->Up());
			func(mCamera->Right());

			o.close();
		}

		delete mCamera;
		delete mSceneHierarchy;
		delete mNodeInspector;
		delete mProjectExplorer;
		delete mConsole;
	}

	void Editor::Execute()
	{
		if (mActiveProject && global::ActiveProjectDir.empty())
		{
			global::ActiveProjectDir = mActiveProject->Directory() + "\\" + mActiveProject->Name() + "\\";
			global::ActiveSceneDir = mActiveProject->ActiveScene()->Directory() + "\\" + mActiveProject->ActiveScene()->Name() + "\\";
			global::ActiveProjectName = mActiveProject->Name();
			global::ActiveSceneName = mActiveProject->ActiveScene()->Name();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 12.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 2.0f);

		auto dockspace_id = ImGui::GetID("Root_Dockspace");

		auto* viewport = ImGui::GetMainViewport();
		static float IFDModalWidth = viewport->Size.x / 2;
		static float IFDModalHeight = viewport->Size.y / 2 + 100.0f;

		//Dockspace
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		static ImGuiWindowFlags host_window_flags = 0;
		host_window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
		host_window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking;
		host_window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Choice DockSpace", (bool*)0, host_window_flags);
		ImGui::PopStyleVar();

		ImGui::PopStyleVar(2);

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), 0);
		}

		//Main Menu Bar
		if (ImGui::BeginMainMenuBar())
		{
			ShowFileMenu();

			ImGui::EndMainMenuBar();
		}//Main Menu Bar


		if (Input::IsKeyPressed(Key::LEFTCONTROL))
		{
			if (Input::IsKeyPressed(Key::N))
			{
				mModalPurpose = ModalPurpose::NEWPROJECT; mShowModal = true;
			}
			if (Input::IsKeyPressed(Key::S))
			{
				if (mActiveProject) { mActiveProject->Save(); }
			}
			if (Input::IsKeyPressed(Key::O))
			{
				std::string cproj = FileDialog::OpenFile("Choice Project (*.cproj)\0*.cproj\0");
				if (!cproj.empty())
				{
					if (mActiveProject) { delete mActiveProject; }
					mActiveProject = new Project(cproj);
					Choice::Instance()->GetWindow()->UpdateTitle(("Choice | " + mActiveProject->Name() + " |").c_str());
				}
			}
		}

		//New Project
		if (mShowModal)
		{
			switch (mModalPurpose)
			{
			case ModalPurpose::NEWPROJECT:
				ImGui::OpenPopup("New Project");
				ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Appearing, { 0.5f, 0.5f });
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
							memcpy(dirbuf, dir.data(), dir.size());
						}
						ImGuiFileDialog::Instance()->Close();
					}

					ImGui::Separator();

					if (ImGui::Button("Create") || Input::IsKeyPressed(Key::ENTER))
					{
						if (strlen(namebuf) == 0) 
						{ 
							mShowModal = Message<ERROR_MSG>("Project Name Can't Be Empty", MESSAGE_ORIGIN::EDITOR);
						}

						if (strlen(dirbuf) == 0) 
						{ 
							mShowModal = Message<ERROR_MSG>("No Directory Selected", MESSAGE_ORIGIN::EDITOR);
						}
						else
						{
							if (!ghc::filesystem::exists(dirbuf))
							{
								std::string msg(dirbuf);
								msg += " ";
								msg += "Is Not A Valid Directory";
								mShowModal = Message<ERROR_MSG>(msg.c_str(), MESSAGE_ORIGIN::FILESYSTEM);
							}
						}
						
						if (mShowModal)
						{
							if (mActiveProject) { delete mActiveProject; }
							mActiveProject = new Project(namebuf, dirbuf);
							memset(namebuf, 0, sizeof(namebuf));
							memset(dirbuf, 0, sizeof(dirbuf));

							Choice::Instance()->GetWindow()->UpdateTitle(("Choice | " + mActiveProject->Name() + " |").c_str());
						}

						mShowModal = false;
						mModalPurpose = ModalPurpose::NONE;
					}
					ImGui::SameLine();
					if (ImGui::Button("Cancel") || Input::IsKeyPressed(Key::ESCAPE))
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
		}//New Project

		if (mActiveProject)
		{
			mSceneHierarchy->Execute(mActiveProject->ActiveScene());
			mNodeInspector->Execute(mSceneHierarchy->SelectedNode());
		}

		//Console
		mConsole->Execute();

		//Viewport
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
		ImGui::PushStyleColor(ImGuiCol_TabUnfocused, { 0.2f, 0.6f, 0.8f, 0.8f });
		ImGui::PushStyleColor(ImGuiCol_TabUnfocusedActive, { 0.2f, 0.6f, 0.8f, 0.8f });
		ImGui::PushStyleColor(ImGuiCol_TabActive, { 0.2f, 0.6f, 0.8f, 0.8f });
		ImGui::Begin(ICON_FA_GAMEPAD" Viewport");

		ImGui::PopStyleColor(3);

		if (ImGui::IsWindowFocused() && ImGui::IsWindowHovered())
		{
			mCamera->AcceptInput(true);
		}

		ImGui::PopStyleVar();

		ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		if (mVisibleRegion != *(glm::vec2*)&viewportSize)
		{
			mVisibleRegion.x = viewportSize.x; mVisibleRegion.y = viewportSize.y;
			Choice::Instance()->GetPipeline()->Visible((uint32_t)mVisibleRegion.x, (uint32_t)mVisibleRegion.y);
			mCamera->Visible((uint32_t)mVisibleRegion.x, (uint32_t)mVisibleRegion.y);
		}

		ImGui::Image((void*)(uintptr_t)Choice::Instance()->GetPipeline()->Capture(), { mVisibleRegion.x, mVisibleRegion.y }, ImVec2{ 0,1 }, ImVec2{ 1,0 });

		if (mActiveProject)
		{
			ShowAddingMenu(mActiveProject->ActiveScene());
		}

		//Gizmo
		if (Input::IsKeyPressed(Key::Q)) { mGizmoType = -1; }
		if (Input::IsKeyPressed(Key::NUM1)) { mGizmoType = ImGuizmo::OPERATION::TRANSLATE; }
		if (Input::IsKeyPressed(Key::NUM2)) { mGizmoType = ImGuizmo::OPERATION::ROTATE; }
		if (Input::IsKeyPressed(Key::NUM3)) { mGizmoType = ImGuizmo::OPERATION::SCALE; }

		if (mSceneHierarchy->SelectedNode() && mGizmoType != -1)
		{
			Node* selectednode = mSceneHierarchy->SelectedNode();

			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);

			bool snap = Input::IsKeyPressed(Key::LEFTCONTROL);
			float snapValue = 0.5f;
			if (mGizmoType == ImGuizmo::OPERATION::ROTATE) { snapValue = 45.0f; }

			float snapValues[3] = { snapValue, snapValue, snapValue };

			Transform* transform = selectednode->NodeTransform;
			glm::mat4 _transform = transform->GetTransform();

			ImGuizmo::Manipulate(glm::value_ptr(mCamera->View()), glm::value_ptr(mCamera->Projection()),
				(ImGuizmo::OPERATION)mGizmoType, ImGuizmo::LOCAL, glm::value_ptr(_transform), nullptr,
				snap ? snapValues : nullptr);

			if (ImGuizmo::IsUsing())
			{
				glm::vec3 rotation;
				DecomposeTransform(_transform, transform->Position, rotation, transform->Scale);
				glm::vec3 deltaRotation = rotation - transform->Rotation;
				transform->Rotation += deltaRotation;
			}
		}//Gizmo

		ImGui::End();//Viewport

		ImGui::End();//Dockspace

		ImGui::PopStyleVar(2);
	}

	void Editor::Update()
	{
		mCamera->Update();
	}

	void Editor::ShowFileMenu()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New Project", "Ctrl + N"))
			{
				mModalPurpose = ModalPurpose::NEWPROJECT; mShowModal = true;
			}

			if (ImGui::MenuItem("Save Project", "Ctrl + S"))
			{
				if (mActiveProject) { mActiveProject->Save(); }
			}

			if (ImGui::MenuItem("Open Project", "Ctrl + O"))
			{
				std::string cproj = FileDialog::OpenFile("Choice Project (*.cproj)\0*.cproj\0");
				if (!cproj.empty())
				{
					if (mActiveProject) { delete mActiveProject; }
					mActiveProject = new Project(cproj);
					Choice::Instance()->GetWindow()->UpdateTitle(("Choice | " + mActiveProject->Name() + " |").c_str());
				}
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Blender", nullptr, mIsBlenderLinked))
			{
#ifdef EXE
				std::string blenderpath = FileDialog::OpenFile("Executable (*.exe)\0*.exe\0");
				if (!blenderpath.empty())
				{
					std::string name = blenderpath.substr(blenderpath.find_last_of('\\') + 1, blenderpath.size() - 1);
					if (name != "blender.exe")
					{
						Message<ERROR_MSG>("Cannot Link Blender", MESSAGE_ORIGIN::EDITOR);
						return;
					}

					mIsBlenderLinked = true;

					char c = '"';
					std::ofstream writebat("gltf.bat", std::ios::out);

					//Absolute Path To gltf.py
					std::string abspath = ghc::filesystem::absolute("Choice/assets/scripts/gltf.py").string();

					writebat << "call " << c + blenderpath + c << " --background --python " << c + abspath + c << std::endl;
					writebat << "cls";

					writebat.close();
				}
#endif
			}
			ImGui::EndMenu();
		}
	}

}