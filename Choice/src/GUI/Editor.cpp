#include "Editor.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuiFileDialog.h>
#include <ImGuizmo.h>

#include "FontAwesome.h"

#include "Input.h"
#include "Choice.h"

#include <glm/glm.hpp>

namespace choice
{
	Editor::Editor(uint32_t w, uint32_t h)
	{
		if (ghc::filesystem::exists("gltf.bat")) { mIsBlenderLinked = true;; }

		std::ifstream o(".choiceeditorconfig", std::ios::in | std::ios::binary);
		if (!o.is_open())
		{ 
			mCamera = new EditorCamera((float)w / (float)h);
			mActiveProject = {};
		}
		else
		{
			uint32_t cprojsize;
			o.read((char*)&cprojsize, sizeof(cprojsize));
			std::string cproj;
			cproj.resize(cprojsize);
			o.read((char*)cproj.data(), cprojsize);

			glm::vec3 focus;
			o.read((char*)&focus.x, sizeof(focus.x));
			o.read((char*)&focus.y, sizeof(focus.y));
			o.read((char*)&focus.z, sizeof(focus.z));

			glm::vec3 offset;
			o.read((char*)&offset.x, sizeof(offset.x));
			o.read((char*)&offset.y, sizeof(offset.y));
			o.read((char*)&offset.z, sizeof(offset.z));

			glm::vec3 up;
			o.read((char*)&up.x, sizeof(up.x));
			o.read((char*)&up.y, sizeof(up.y));
			o.read((char*)&up.z, sizeof(up.z));

			glm::vec3 right;
			o.read((char*)&right.x, sizeof(right.x));
			o.read((char*)&right.y, sizeof(right.y));
			o.read((char*)&right.z, sizeof(right.z));

			mCamera = new EditorCamera((float)w / (float)h, focus, offset, up, right);

			if (!ghc::filesystem::exists(cproj)) { mActiveProject = {}; }
			else { mActiveProject = std::make_unique<Project>(cproj); }
		}
		
		o.close();
	}

	Editor::~Editor()
	{
		//Write down the active project And Camera Data
		std::ofstream o(".choiceeditorconfig", std::ios::out | std::ios::binary);
		if (mActiveProject)
		{
			std::string cproj = mActiveProject->Directory() + "\\" + mActiveProject->Name() + "\\" + mActiveProject->Name() + ".cproj";
			uint32_t activeprojectsize = (uint32_t)cproj.size();
			o.write((char*)&activeprojectsize, sizeof(activeprojectsize));
			o.write((char*)cproj.data(), activeprojectsize);

			o.write((char*)&mCamera->Focus().x, sizeof(mCamera->Focus().x));
			o.write((char*)&mCamera->Focus().y, sizeof(mCamera->Focus().y));
			o.write((char*)&mCamera->Focus().z, sizeof(mCamera->Focus().z));

			o.write((char*)&mCamera->Offset().x, sizeof(mCamera->Offset().x));
			o.write((char*)&mCamera->Offset().y, sizeof(mCamera->Offset().y));
			o.write((char*)&mCamera->Offset().z, sizeof(mCamera->Offset().z));

			o.write((char*)&mCamera->Up().x, sizeof(mCamera->Up().x));
			o.write((char*)&mCamera->Up().y, sizeof(mCamera->Up().y));
			o.write((char*)&mCamera->Up().z, sizeof(mCamera->Up().z));

			o.write((char*)&mCamera->Right().x, sizeof(mCamera->Right().x));
			o.write((char*)&mCamera->Right().y, sizeof(mCamera->Right().y));
			o.write((char*)&mCamera->Right().z, sizeof(mCamera->Right().z));
		}
		o.close();

		delete mCamera;
	}

	void Editor::Execute()
	{
		//SetEditorLayout();

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
		host_window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking;
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

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem("Project Explorer", "Alt + P"))
				{
					mShowProjectExplorer = true;
				}
				ImGui::EndMenu();
			}

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
				ImGuiFileDialog::Instance()->SetExtentionInfos(".cproj", { 1.0f, 0.0f, 1.0f, 1.0f });
				ImGuiFileDialog::Instance()->OpenModal("OpenProject", "Open Project", ".cproj", "");
			}
		}

		if (Input::IsKeyPressed(Key::LEFTALT) && Input::IsKeyPressed(Key::P)) { mShowProjectExplorer = true; }

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
		}//New Project


		//Open Project
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
		}//Open Project


		//Link Blender
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
		}//Link Blender


		//Load Model
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
				std::string srcFile = DumpDrawable(modelfilepath.substr(0, modelfilepath.find_last_of('.')) + ".glb",
					directory, DrawableType::MODEL);

				if (!(ext == ".glb" || ext == ".gltf"))
				{
					ghc::filesystem::remove(modelfilepath.substr(0, modelfilepath.find_last_of('.')) + ".glb");
				}

				SceneObject* sceneobject = new SceneObject();
				Drawable* drawable = LoadDrawable(srcFile, DrawableType::MODEL, true);
				if (drawable)
				{
					sceneobject->AddProperty<Drawable>(drawable);
					Transform* transform = new Transform();
					transform->Position = { 0.0f, 0.0f, 0.0f };
					transform->Rotation = { glm::radians(90.0f), 0.0f, 0.0f };
					transform->Scale = { 1.0f, 1.0f, 1.0f };
					sceneobject->AddProperty<Transform>(transform);

					mActiveProject->ActiveScene()->AddObject(sceneobject);
				}
				else
				{
					delete sceneobject;
				}
			}
			ImGuiFileDialog::Instance()->Close();
		}//Load Model

		//Change Skybox
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
		}//Change Skybox


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

		ShowAddingScneObjectMenu();

		//Gizmo
		if (Input::IsKeyPressed(Key::Q)) { mGizmoType = -1; }
		if (Input::IsKeyPressed(Key::NUM1)) { mGizmoType = ImGuizmo::OPERATION::TRANSLATE; }
		if (Input::IsKeyPressed(Key::NUM2)) { mGizmoType = ImGuizmo::OPERATION::ROTATE; }
		if (Input::IsKeyPressed(Key::NUM3)) { mGizmoType = ImGuizmo::OPERATION::SCALE; }

		if (mSelectedObjectIndex != -1 && mGizmoType != -1)
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
		}//Gizmo

		ImGui::End();//Viewport


		//Scene Hierarchy
		ImGui::Begin(ICON_FK_LIST_UL" Hierarchy");

		if (ImGui::IsWindowFocused() && ImGui::IsWindowHovered()) { Choice::Instance()->GetPipeline()->MousePicking(false); }

		if (mActiveProject)
		{
			std::string icon = ICON_FK_PICTURE_O;
			if (ImGui::CollapsingHeader((icon + " " + mActiveProject->ActiveScene()->Name()).c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen))
			{
				for (uint32_t i = 0; i < mActiveProject->ActiveScene()->GetSceneObjects().size(); i++)
				{
					SceneObject* object = mActiveProject->ActiveScene()->GetSceneObjects()[i];
					if (object)
					{
						ImGuiTreeNodeFlags _flags_ = (i == mSelectedObjectIndex) ? ImGuiTreeNodeFlags_Selected : 0;
						_flags_ |= ImGuiTreeNodeFlags_Leaf;
						if (object->GetProperty<Skybox>()) { icon = ICON_FK_SKYATLAS; }
						else if (object->GetProperty<Drawable>()) { icon = ICON_FK_CUBE; }
						else if (object->GetProperty<Light>()) { icon = ICON_FK_LIGHTBULB_O; }
						if (ImGui::TreeNodeEx((icon + " " + object->Name()).c_str(), _flags_))
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

			ShowAddingScneObjectMenu();
		}
		ImGui::End();//Scene Hierarchy


		//Object Inspector
		if (mSelectedObjectIndex != -1)
		{
			SceneObject* object = mActiveProject->ActiveScene()->GetSceneObjects()[mSelectedObjectIndex];
			if (object) { DrawObjectInspectorPanel(object); }
		}//Object Inspector


		//Project Explorer
		if (mShowProjectExplorer)
		{
			DrawProjectExplorer();
		}//Project Explorer

		ImGui::End();//Dockspace
	}

	void Editor::Update()
	{
		mCamera->Update();
	}

	template<typename T>
	void SceneObject::DrawProperty() { static_assert(false); }

	template<>
	void SceneObject::DrawProperty<Drawable>()
	{
		if (mDrawable.has_value())
		{
			if (ImGui::CollapsingHeader(ICON_FK_CUBE" Drawable", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen))
			{
				static bool showMaterialsInterface = true;
				ImGui::Text("Show Materials Interface");
				ImGui::SameLine();
				ImGui::Checkbox("##SMI", &showMaterialsInterface);
				if (showMaterialsInterface)
				{
					ImGui::Separator();

					std::vector<const char*> comboitems;
					if (comboitems.size() != mDrawable.value()->GetMaterials().size())
					{
						for (auto& material : mDrawable.value()->GetMaterials())
						{
							comboitems.push_back(material->Name.c_str());
						}
					}

					ImGui::Text("Materials  ");
					ImGui::SameLine();
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
					static int currentitem = 0;
					ImGui::Combo("##Materials", &currentitem, comboitems.data(), comboitems.size());

					Material* material = mDrawable.value()->GetMaterials()[currentitem];

					static bool isDiffuseClicked = false;
					static bool isNormalClicked = false;

					std::string name = "Albedo";

					if (material->DiffuseMap.second.second)
					{
						name += " - ";
						name += ghc::filesystem::path(ghc::filesystem::path(material->DiffuseMap.second.second->Source).stem().string()).stem().string();
					}

					if (ImGui::TreeNode(name.c_str()))
					{
						void* id = {};
						if (material->DiffuseMap.second.first) { id = (void*)(uintptr_t)material->DiffuseMap.second.first->GetId(); }

						if (ImGui::ImageButton(id, { 50.0f, 50.0f }))
						{
							isDiffuseClicked = true; isNormalClicked = false;
							ImGuiFileDialog::Instance()->SetExtentionInfos(".png", { 0.5f, 1.0f, 0.5f, 1.0f });
							ImGuiFileDialog::Instance()->SetExtentionInfos(".jpg", { 0.5f, 0.5f, 1.0f, 1.0f });
							ImGuiFileDialog::Instance()->OpenModal("OpenTexture", "Open Texture", ".png,.jpg", "");
						}
						if (material->DiffuseMap.second.first)
						{
							ImGui::SameLine();
							ImGui::Checkbox("##UseAlbedo", &material->DiffuseMap.first);
						}

						ImGui::SameLine();
						ImGui::ColorEdit4("##Color", glm::value_ptr(mDrawable.value()->GetMaterials()[currentitem]->Color),
							ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoInputs
							| ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoBorder);

						ImGui::TreePop();
					}

					name = "Normal";

					if (material->NormalMap.second.second)
					{
						name += " - ";
						name += ghc::filesystem::path(ghc::filesystem::path(material->NormalMap.second.second->Source).stem().string()).stem().string();
					}

					if (ImGui::TreeNode(name.c_str()))
					{
						void* id = {};
						if (material->NormalMap.second.first) { id = (void*)(uintptr_t)material->NormalMap.second.first->GetId(); }
						if (ImGui::ImageButton(id, { 50.0f, 50.0f }))
						{
							isNormalClicked = true; isDiffuseClicked = false;
							ImGuiFileDialog::Instance()->SetExtentionInfos(".png", { 0.5f, 1.0f, 0.5f, 1.0f });
							ImGuiFileDialog::Instance()->SetExtentionInfos(".jpg", { 0.5f, 0.5f, 1.0f, 1.0f });
							ImGuiFileDialog::Instance()->OpenModal("OpenTexture", "Open Texture", ".png,.jpg", "");
						}
						if (material->NormalMap.second.first)
						{
							ImGui::SameLine();
							ImGui::Checkbox("##UseNormal", &material->NormalMap.first);
						}

						ImGui::TreePop();
					}

					ImGui::SliderFloat("Roughness", &material->Roughness, 0.0f, 1.0f);
					ImGui::SliderFloat("Metallic", &material->Metallic, 0.0f, 1.0f);

					//Open Texture
					if (ImGuiFileDialog::Instance()->Display("OpenTexture", ImGuiWindowFlags_NoCollapse,
						{ ImGui::GetMainViewport()->WorkSize.x / 2, ImGui::GetMainViewport()->WorkSize.y / 2 + 100.0f }))
					{
						if (ImGuiFileDialog::Instance()->IsOk())
						{
							std::string texture = ImGuiFileDialog::Instance()->GetFilePathName();
							Texture2DData* data = new Texture2DData();
							
							BlockCompressionFormat format;
							if (isDiffuseClicked) { format = BlockCompressionFormat::BC1; }
							else if (isNormalClicked) { format = BlockCompressionFormat::BC5; }

							std::string dstDirectory = Choice::Instance()->GetEditor()->GetActiveProject()->ActiveScene()->Directory() + "\\" +
								Choice::Instance()->GetEditor()->GetActiveProject()->ActiveScene()->Name() + "\\Assets";

							data->Source = CompressTexture(texture, dstDirectory, format, false);
							data->magFilter = (uint32_t)GL_LINEAR;
							data->minFilter = (uint32_t)GL_LINEAR;
							data->wrapS = (uint32_t)GL_REPEAT;
							data->wrapT = (uint32_t)GL_REPEAT;

							if (isDiffuseClicked)
							{
								if (material->DiffuseMap.second.first) { delete material->DiffuseMap.second.first; }
								material->DiffuseMap.second.first = new Texture2D(LoadTexture2D(*data));

								if (material->DiffuseMap.second.second) { delete material->DiffuseMap.second.second; }
								material->DiffuseMap.second.second = data;
							}
							else if (isNormalClicked)
							{
								if (material->NormalMap.second.first) { delete material->NormalMap.second.first; }
								material->NormalMap.second.first = new Texture2D(LoadTexture2D(*data));

								if (material->NormalMap.second.second) { delete material->NormalMap.second.second; }
								material->NormalMap.second.second = data;
							}
						}
						ImGuiFileDialog::Instance()->Close();
					}//Open Texture
				}
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
		ImGui::SetNextItemWidth(53.0f);
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
		ImGui::SetNextItemWidth(53.0f);
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
		ImGui::SetNextItemWidth(53.0f);
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
					TransformUI("Position ", mTransform.value()->Position, 0.0f);
					ImGui::TableNextRow();
					glm::vec3 rotation = glm::degrees(mTransform.value()->Rotation);
					TransformUI("Rotation ", rotation, 0.0f);
					mTransform.value()->Rotation = glm::radians(rotation);
					ImGui::TableNextRow();
					TransformUI("Scale    ", mTransform.value()->Scale, 1.0f);
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

	template<>
	void SceneObject::DrawProperty<Light>()
	{
		if (mLight.has_value())
		{
			if (ImGui::CollapsingHeader(ICON_FK_LIGHTBULB_O" Light", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Text("Type         ");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
				const char* comboitems[] = { "Directional Light", "Point Light" };
				static int type = static_cast<int>(mLight.value()->GetLightType());
				ImGui::Combo("##Type", &type, comboitems, IM_ARRAYSIZE(comboitems));

				ImGui::Text("Color        ");
				ImGui::SameLine();
				ImGui::ColorEdit3("##Color", glm::value_ptr(mLight.value()->GetDiffuse()), ImGuiColorEditFlags_PickerHueWheel);
			
				ImGui::Text("Intensity    ");
				ImGui::SameLine();
				ImGui::DragFloat("##Intensity", &mLight.value()->GetIntensity(), 0.2f, 0.0f, 10.0f);

				if (mLight.value()->GetLightType() == LightType::POINT)
				{
					ImGui::Text("Radius      ");
					ImGui::SameLine();
					ImGui::DragFloat("##Radius", &mLight.value()->GetRadius(), 0.2f, 0.0f, 100.0f);
				}
			}
		}
	}

	void Editor::DrawObjectInspectorPanel(SceneObject* object)
	{
		ImGui::Begin(ICON_FK_INFO_CIRCLE" Inspector");

		if (ImGui::IsWindowFocused())
		{
			Choice::Instance()->GetPipeline()->MousePicking(false);
		}

		ImGui::Text(("Name :" + object->Name()).c_str());
		ImGui::Button("Rename TODO");
		ImGui::SameLine();
		ImGui::Button("Delete TODO");

		object->DrawProperty<Transform>();
		object->DrawProperty<Drawable>();
		object->DrawProperty<Skybox>();
		object->DrawProperty<Light>();

		ImGui::End();
	}

	void Editor::SetEditorLayout()
	{
		if (ImGui::DockBuilderGetNode(mDockIds.root) == NULL) {
			mDockIds.root = ImGui::GetID("Root_Dockspace");

			ImGui::DockBuilderRemoveNode(mDockIds.root);  // Clear out existing layout
			ImGui::DockBuilderAddNode(mDockIds.root,
				ImGuiDockNodeFlags_DockSpace);  // Add empty node
			ImGui::DockBuilderSetNodeSize(mDockIds.root,
				{ (float)Choice::Instance()->GetWindow()->GetWidth(), 
				(float)Choice::Instance()->GetWindow()->GetHeight() });

			mDockIds.right = ImGui::DockBuilderSplitNode(mDockIds.root, ImGuiDir_Right,
				0.2f, NULL, &mDockIds.root);
			mDockIds.right_bottom = ImGui::DockBuilderSplitNode(mDockIds.right, ImGuiDir_Down,
				0.2f, NULL, &mDockIds.right);
			mDockIds.left = ImGui::DockBuilderSplitNode(mDockIds.root, ImGuiDir_Left,
				0.2f, NULL, &mDockIds.root);

			ImGui::DockBuilderFinish(mDockIds.root);
		}
	}

	//Iterate Project Directory
	void IterateDirectory(const std::string& directory)
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
	}//Iterate Project Directory

	//Project Explorer
	void Editor::DrawProjectExplorer()
	{
		ImGui::SetNextWindowDockID(mDockIds.left, ImGuiCond_Appearing);
		ImGui::Begin(ICON_FK_FOLDER_OPEN_O" Explorer", &mShowProjectExplorer);

		std::string icon = ICON_FK_FOLDER_OPEN;
		if (ImGui::TreeNodeEx((icon + " " + mActiveProject->Name()).c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Leaf))
		{
			IterateDirectory(mActiveProject->Directory() + "\\" + mActiveProject->Name()); //Iterate Project Directory
			ImGui::TreePop();
		}

		ImGui::End();
	}//Project Explorer

	void Editor::ShowAddingScneObjectMenu()
	{
		if (ImGui::BeginPopupContextWindow(0, 1, false))
		{
			if (ImGui::BeginMenu("Add Drawable"))
			{
				//Add Cube
				if (ImGui::MenuItem("Cube"))
				{
					SceneObject* sceneobject = new SceneObject();
					sceneobject->AddProperty<Drawable>(LoadDrawable("Cube", DrawableType::CUBE, true));
					Transform* transform = new Transform();
					transform->Position = { 0.0f, 0.0f, 0.0f };
					transform->Rotation = { 0.0f, 0.0f, 0.0f };
					transform->Scale = { 1.0f, 1.0f, 1.0f };
					sceneobject->AddProperty<Transform>(transform);
					mActiveProject->ActiveScene()->AddObject(sceneobject);
				}

				//Add Sphere
				if (ImGui::MenuItem("Sphere"))
				{
					SceneObject* sceneobject = new SceneObject();
					sceneobject->AddProperty<Drawable>(LoadDrawable("Sphere", DrawableType::SPHERE, true));
					Transform* transform = new Transform();
					transform->Position = { 0.0f, 0.0f, 0.0f };
					transform->Rotation = { 0.0f, 0.0f, 0.0f };
					transform->Scale = { 1.0f, 1.0f, 1.0f };
					sceneobject->AddProperty<Transform>(transform);
					mActiveProject->ActiveScene()->AddObject(sceneobject);
				}

				//Add Model
				if (ImGui::MenuItem("Model"))
				{
					ImGuiFileDialog::Instance()->SetExtentionInfos(".obj", { 0.1f, 1.0f, 0.1f, 1.0f });
					ImGuiFileDialog::Instance()->OpenModal("AddModel", "Import Model", ".obj", "");
				}
				
				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("Change Skybox"))
			{
				ImGuiFileDialog::Instance()->SetExtentionInfos(".hdr", { 0.4f, 0.5f, 0.7f, 1.0f });
				ImGuiFileDialog::Instance()->SetExtentionInfos(".exr", { 0.5f, 0.2f, 0.5f, 1.0f });
				ImGuiFileDialog::Instance()->OpenModal("ChangeSkybox", "Change Skybox", ".hdr,.exr", "");
			}
			if (ImGui::BeginMenu("Add Light"))
			{
				if (ImGui::MenuItem("Directional Light"))
				{
					SceneObject* sceneobject = new SceneObject();
					sceneobject->AddProperty<Light>(new DirectionalLight());
					Transform* transform = new Transform();
					transform->Position = { 0.0f, 0.0f, 0.0f };
					transform->Rotation = { 0.0f, 0.0f, 0.0f };
					transform->Scale = { 1.0f, 1.0f, 1.0f };
					sceneobject->AddProperty<Transform>(transform);
					mActiveProject->ActiveScene()->AddObject(sceneobject);
				}
				if (ImGui::MenuItem("Point Light"))
				{
					SceneObject* sceneobject = new SceneObject();
					sceneobject->AddProperty<Light>(new PointLight());
					Transform* transform = new Transform();
					transform->Position = { 0.0f, 0.0f, 0.0f };
					transform->Rotation = { 0.0f, 0.0f, 0.0f };
					transform->Scale = { 1.0f, 1.0f, 1.0f };
					sceneobject->AddProperty<Transform>(transform);
					mActiveProject->ActiveScene()->AddObject(sceneobject);
				}
				ImGui::EndMenu();
			}
			ImGui::EndPopup();
		}
	}

}