#include "Editor.h"

#include <imgui.h>

#include <glad/glad.h>

namespace choice
{
	Editor::Editor(uint32_t w, uint32_t h)
	{
		mCamera = std::make_unique<EditorCamera>((float)w / (float)h);

		mShader = std::make_unique<Shader>("Choice/assets/shaders/Test.glsl");
		mModel = LoadModel("E:/untitled.glb");
	}

	Editor::~Editor()
	{
		delete mModel;
	}

	void Editor::Execute()
	{
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
		
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Blender"))
				{

				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}

	void Editor::Update()
	{
		for (auto& mesh : mModel->Meshes)
		{
			mShader->Use();
			mShader->Mat4("uViewProjection", mCamera->ViewProjection());
			mShader->Mat4("uTransform", glm::mat4(1.0f));
			mesh.first->Bind();
			uint32_t count = (uint32_t)mesh.first->GetCount();
			glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
		}

		mCamera->Update();
	}

}