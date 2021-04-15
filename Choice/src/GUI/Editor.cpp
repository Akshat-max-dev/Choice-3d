#include "Editor.h"

#include <imgui.h>
#include <ImGuiFileDialog.h>

#include "FontAwesome.h"

#include "Input.h"
#include "Choice.h"

#include <glm/glm.hpp>

namespace choice
{
	Editor::Editor(uint32_t w, uint32_t h)
	{
		mCamera = new EditorCamera((float)w / (float)h);
		std::ifstream checkblenderlink("gltf.bat", std::ios::in);
		if (checkblenderlink.is_open()) { mIsBlenderLinked = true; }
		checkblenderlink.close();
		mActiveProject = {};
	}

	Editor::~Editor()
	{
		delete mCamera;
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
					if (mActiveProject) { mActiveProject->Save(); }
				}

				if (ImGui::MenuItem("Open Project", "Ctrl + O"))
				{
					ImGuiFileDialog::Instance()->SetExtentionInfos(".cproj", { 1.0f, 0.0f, 1.0f, 1.0f });
					ImGuiFileDialog::Instance()->OpenModal("OpenProject", "Open Project", ".cproj", "");
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Blender", nullptr, mIsBlenderLinked))
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
				ImGuiFileDialog::Instance()->SetExtentionInfos(".cproj", { 1.0f, 0.0f, 1.0f, 1.0f });
				ImGuiFileDialog::Instance()->OpenModal("OpenProject", "Open Project", ".cproj", "");
			}
		}

		if (mActiveProject)
		{
			if (ImGui::BeginPopupContextVoid())
			{
				if (ImGui::MenuItem("Add Model"))
				{
					ImGuiFileDialog::Instance()->SetExtentionInfos(".obj", { 0.1f, 1.0f, 0.1f, 1.0f });
					ImGuiFileDialog::Instance()->OpenModal("AddModel", "Import Model", ".obj,.glb", "");
				}
				if (ImGui::MenuItem("Change Skybox"))
				{
					ImGuiFileDialog::Instance()->SetExtentionInfos(".hdr", { 0.4f, 0.5f, 0.7f, 1.0f });
					ImGuiFileDialog::Instance()->SetExtentionInfos(".exr", { 0.5f, 0.2f, 0.5f, 1.0f });
					ImGuiFileDialog::Instance()->OpenModal("ChangeSkybox", "Change Skybox", ".hdr,.exr", "");
				}
				ImGui::EndPopup();
			}
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

						if (mActiveProject) { mActiveProject.reset(); }
						mActiveProject = std::make_unique<Project>(namebuf, dirbuf);
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
				std::string abspath = ghc::filesystem::absolute("Choice/assets/scripts/gltf.py").string();

				writebat << "call " << c + path + c << " --background --python " << c + abspath + c << std::endl;
				writebat << "cls";

				writebat.close();
#endif
			}
			ImGuiFileDialog::Instance()->Close();
		}

		if (ImGuiFileDialog::Instance()->Display("AddModel", ImGuiWindowFlags_NoCollapse, { IFDModalWidth, IFDModalHeight }))
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

				std::string ext = ghc::filesystem::path(modelfilepath).extension().string();

				if (!(ext == ".glb" || ext == ".gltf"))
				{
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
					ghc::filesystem::remove("Temporary");
				}
				
				std::string directory = mActiveProject->ActiveScene()->Directory() + "\\" +
					mActiveProject->ActiveScene()->Name() + "\\" + "Assets";
				std::string srcFile = DumpModel(modelfilepath.substr(0, modelfilepath.find_last_of('.')) + ".glb", 
					directory);

				if (!(ext == ".glb" || ext == ".gltf"))
				{
					ghc::filesystem::remove(modelfilepath.substr(0, modelfilepath.find_last_of('.')) + ".glb");
				}

				SceneObject* sceneobject = new SceneObject();
				Model* model = LoadModel(srcFile);
				if (model)
				{
					sceneobject->AddProperty<Model>(model);
					if (ghc::filesystem::exists(directory + "\\Temp"))
					{
						sceneobject->AddProperty<Transform>(LoadModelTransform(directory + "\\Temp"));
						ghc::filesystem::remove(directory + "\\Temp");
					}
					else
					{
						Transform* transform = new Transform();
						transform->Position = { 0.0f, 0.0f, 0.0f };
						transform->Rotation = { 0.0f, 0.0f, 0.0f };
						transform->Scale = { 1.0f, 1.0f, 1.0f };
						sceneobject->AddProperty<Transform>(transform);
					}

					mActiveProject->ActiveScene()->AddObject(sceneobject);
				}
				else
				{
					delete sceneobject;
				}
			}
			ImGuiFileDialog::Instance()->Close();
		}

		if (ImGuiFileDialog::Instance()->Display("ChangeSkybox", ImGuiWindowFlags_NoCollapse, { IFDModalWidth, IFDModalHeight }))
		{
			if (ImGuiFileDialog::Instance()->IsOk())
			{
				std::string hdri = ImGuiFileDialog::Instance()->GetFilePathName();

				std::string hdriname = hdri.substr(hdri.find_last_of('\\') + 1, hdri.size());

				std::string dsthdri = mActiveProject->ActiveScene()->Directory() + "\\" +
					mActiveProject->ActiveScene()->Name() + "\\" + "Assets\\" + hdriname;

				ghc::filesystem::copy_file(hdri, dsthdri);
				ghc::filesystem::remove(mActiveProject->ActiveScene()->GetSceneObjects()[0]->GetProperty<Skybox>()->GetFilepath());
				mActiveProject->ActiveScene()->DeleteObject(0);

				mActiveProject->ActiveScene()->GetSceneObjects()[0] = new SceneObject();
				mActiveProject->ActiveScene()->GetSceneObjects()[0]->AddProperty<Skybox>(new Skybox(dsthdri));
			}
			ImGuiFileDialog::Instance()->Close();
		}

		if (mSelectedObjectIndex != -1)
		{
			SceneObject* object = mActiveProject->ActiveScene()->GetSceneObjects()[mSelectedObjectIndex];
			if (object) { DrawObjectInspectorPanel(object); }
		}

		if (Input::IsKeyPressed(Key::LEFTALT) && Input::IsKeyPressed(Key::I) && mActiveProject)
		{
			mShowHiearchy = true;
		}

		if (mShowHiearchy)
		{
			ImGui::SetNextWindowBgAlpha(0.7f);
			ImGui::Begin(ICON_FK_LIST_ALT" Hiearchy", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking);

			if (ImGui::IsWindowFocused())
			{
				if (Input::IsKeyPressed(Key::ESCAPE)) { mShowHiearchy = false; Choice::Instance()->GetPipeline()->MousePicking(true); }
				else { Choice::Instance()->GetPipeline()->MousePicking(false); }
			}
			else if (Input::IsKeyPressed(Mouse::BUTTON1)) { Choice::Instance()->GetPipeline()->MousePicking(true); }

			if (ImGui::CollapsingHeader(mActiveProject->ActiveScene()->Name().c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen))
			{
				for (uint32_t i = 0; i < mActiveProject->ActiveScene()->GetSceneObjects().size(); i++)
				{
					SceneObject* object = mActiveProject->ActiveScene()->GetSceneObjects()[i];
					if (object)
					{
						ImGuiTreeNodeFlags _flags_ = (i == mSelectedObjectIndex) ? ImGuiTreeNodeFlags_Selected : 0;
						_flags_ |= ImGuiTreeNodeFlags_Leaf;
						if (ImGui::TreeNodeEx(object->Name().c_str(), _flags_))
						{
							if (ImGui::IsItemClicked())
							{
								mSelectedObjectIndex = i;
								Choice::Instance()->GetPipeline()->PickedObject(static_cast<int>(i));
							}
							ImGui::TreePop();
						}
					}
				}
			}
			ImGui::End();
		}
	}

	void Editor::Update()
	{
		mCamera->Update();
	}

	template<typename T>
	void SceneObject::DrawProperty() { static_assert(false); }

	template<>
	void SceneObject::DrawProperty<Model>()
	{
		if (mModel.has_value())
		{
			if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_OpenOnArrow))
			{
				ImGui::Text("TODO");
			}
		}
	}

	static void TransformUI(const std::string& label, glm::vec3& value, float resetValue)
	{
		ImGui::PushID(label.c_str());
		ImGui::TableSetColumnIndex(0);
		ImGui::Text(label.c_str());

		ImGui::TableSetColumnIndex(1);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.1f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 1.0f, 0.1f, 0.1f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.1f, 1.0f });
		if (ImGui::Button("X"))
		{
			value.x = resetValue;
		}
		ImGui::PopStyleColor(3);

		ImGui::TableSetColumnIndex(2);
		ImGui::SetNextItemWidth(47.0f);
		ImGui::DragFloat("##X", &value.x, 0.2f, 0.0f, 0.0f, "%.2f");

		ImGui::TableSetColumnIndex(3);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.8f, 0.1f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.1f, 1.0f, 0.1f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.8f, 0.1f, 1.0f });
		if (ImGui::Button("Y"))
		{
			value.y = resetValue;
		}
		ImGui::PopStyleColor(3);

		ImGui::TableSetColumnIndex(4);
		ImGui::SetNextItemWidth(47.0f);
		ImGui::DragFloat("##Y", &value.y, 0.2f, 0.0f, 0.0f, "%.2f");

		ImGui::TableSetColumnIndex(5);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.1f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.1f, 0.1f, 1.0f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.1f, 0.8f, 1.0f });
		if (ImGui::Button("Z"))
		{
			value.z = resetValue;
		}
		ImGui::PopStyleColor(3);

		ImGui::TableSetColumnIndex(6);
		ImGui::SetNextItemWidth(47.0f);
		ImGui::DragFloat("##Z", &value.z, 0.2f, 0.0f, 0.0f, "%.2f");
		ImGui::PopID();
	}

	template<>
	void SceneObject::DrawProperty<Transform>()
	{
		if (mTransform.has_value())
		{
			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow))
			{
				if (ImGui::BeginTable("##Transform", 7, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit))
				{
					ImGui::TableNextRow();
					TransformUI("Position    ", mTransform.value()->Position, 0.0f);
					ImGui::TableNextRow();
					glm::vec3 rotation = glm::degrees(mTransform.value()->Rotation);
					TransformUI("Rotation    ", rotation, 0.0f);
					mTransform.value()->Rotation = glm::radians(rotation);
					ImGui::TableNextRow();
					TransformUI("Scale       ", mTransform.value()->Scale, 1.0f);
					ImGui::EndTable();
				}
			}
		}
	}

	template<>
	void SceneObject::DrawProperty<Skybox>()
	{
		if (mSkybox.has_value())
		{

		}
	}

	void Editor::DrawObjectInspectorPanel(SceneObject* object)
	{
		std::string icon = ICON_FK_INFO_CIRCLE;
		ImGui::SetNextWindowSize({ 343.0f, 289.0f }, ImGuiCond_Appearing);
		ImGui::SetNextWindowSizeConstraints({ 343.0f, 289.0f }, { 343.0f, 679.0f });
		ImGui::SetNextWindowBgAlpha(0.68f);
		ImGui::Begin((icon + " Inspector").c_str(), NULL, ImGuiWindowFlags_NoDocking);

		if (ImGui::IsWindowFocused())
		{
			Choice::Instance()->GetPipeline()->MousePicking(false);
		}
		else if (Input::IsButtonPressed(Mouse::BUTTON1))
		{
			Choice::Instance()->GetPipeline()->MousePicking(true);
		}

		ImGui::Text(("Name :" + object->Name()).c_str());
		ImGui::Button("Rename TODO");
		ImGui::SameLine();
		ImGui::Button("Delete TODO");

		object->DrawProperty<Transform>();
		object->DrawProperty<Model>();
		object->DrawProperty<Skybox>();

		ImGui::End();
	}

}