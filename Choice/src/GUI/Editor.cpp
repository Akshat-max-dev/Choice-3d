#include "Editor.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuiFileDialog.h>
#include <ImGuizmo.h>

#include "FontAwesome.h"

#include "Input.h"
#include "Choice.h"
#include "BinaryHelper.h"
#include "Project/Scene/Nodes/Mesh.h"
#include "FileDialog.h"

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
			Binary::Read<std::string>(file, cproj);

			glm::vec3 focus;
			Binary::Read<glm::vec3>(file, focus);

			glm::vec3 offset;
			Binary::Read<glm::vec3>(file, offset);

			glm::vec3 up;
			Binary::Read<glm::vec3>(file, up);

			glm::vec3 right;
			Binary::Read<glm::vec3>(file, right);

			mCamera = new EditorCamera((float)w / (float)h, focus, offset, up, right);

			if (!ghc::filesystem::exists(cproj)) { mActiveProject = {}; }
			else { mActiveProject = new Project(cproj); }

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
	}

	Editor::~Editor()
	{
		//Write down the active project And Camera Data
		std::ofstream o(".choiceeditorconfig", std::ios::out | std::ios::binary);
		if (mActiveProject)
		{
			std::string cproj = mActiveProject->Directory() + "\\" + mActiveProject->Name() + "\\" + mActiveProject->Name() + ".cproj";
			Binary::Write<std::string>(o, cproj);

			glm::vec3 t = mCamera->Focus();
			Binary::Write<glm::vec3>(o, t);

			t = mCamera->Offset();
			Binary::Write<glm::vec3>(o, t);

			t = mCamera->Up();
			Binary::Write<glm::vec3>(o, t);

			t = mCamera->Right();
			Binary::Write<glm::vec3>(o, t);
		}
		o.close();

		delete mCamera;
		delete mSceneHierarchy;
		delete mNodeInspector;
		delete mProjectExplorer;
	}

	void Editor::Execute()
	{
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
							for (uint32_t i = 0; i < dir.size(); i++)
							{
								dirbuf[i] = dir[i];
							}
						}
						ImGuiFileDialog::Instance()->Close();
					}

					ImGui::Separator();

					if (ImGui::Button("Create") || Input::IsKeyPressed(Key::ENTER))
					{
						if (strlen(namebuf) == 0) { std::cout << "Project Name Cant Be Empty" << std::endl; return; }
						if (strlen(dirbuf) == 0) { std::cout << "No Directory Selected" << std::endl; return; }

						if (mActiveProject) { delete mActiveProject; }
						mActiveProject = new Project(namebuf, dirbuf);
						memset(namebuf, 0, sizeof(namebuf));
						memset(dirbuf, 0, sizeof(dirbuf));

						Choice::Instance()->GetWindow()->UpdateTitle(("Choice | " + mActiveProject->Name() + " |").c_str());

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

		//Viewport
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
		ImGui::Begin(ICON_FK_GAMEPAD" Viewport");

		if (ImGui::IsWindowFocused() && ImGui::IsWindowHovered())
		{
			Choice::Instance()->GetPipeline()->MousePicking(true);
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

		/*if (mSelectedObjectIndex != -1 && mGizmoType != -1)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);

			bool snap = Input::IsKeyPressed(Key::LEFTCONTROL);
			float snapValue = 0.5f;
			if (mGizmoType == ImGuizmo::OPERATION::ROTATE) { snapValue = 45.0f; }

			float snapValues[3] = { snapValue, snapValue, snapValue };

			Transform* transform = mActiveProject->ActiveScene()->GetSceneObjects()[mSelectedObjectIndex]->GetProperty<Transform>();
			glm::mat4 _transform = transform->GetTransform();

			ImGuizmo::Manipulate(glm::value_ptr(mCamera->View()), glm::value_ptr(mCamera->Projection()),
				(ImGuizmo::OPERATION)mGizmoType, ImGuizmo::LOCAL, glm::value_ptr(_transform), nullptr,
				snap ? snapValues : nullptr);
			if (ImGuizmo::IsOver())
			{
				Choice::Instance()->GetPipeline()->MousePicking(false);
			}
			if (ImGuizmo::IsUsing())
			{
				Choice::Instance()->GetPipeline()->MousePicking(false);
				glm::vec3 rotation;
				DecomposeTransform(_transform, transform->Position, rotation, transform->Scale);
				glm::vec3 deltaRotation = rotation - transform->Rotation;
				transform->Rotation += deltaRotation;
			}
		}//Gizmo*/

		ImGui::End();//Viewport

		if (mActiveProject)
		{
			mSceneHierarchy->Execute(mActiveProject->ActiveScene());
			mNodeInspector->Execute(mSceneHierarchy->SelectedNode());
		}

		ImGui::End();//Dockspace
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
						std::cout << "The Selected File Is Not blender.exe" << std::endl;
						choiceassert(0);
					}

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