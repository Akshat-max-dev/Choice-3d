#pragma once
#include "cpch.h"

#include "Camera/Camera.h"
#include "Camera/EditorCamera.h"

#include "Project/Project.h"

namespace choice
{
	class Editor
	{
	public:
		Editor(uint32_t w, uint32_t h);
		~Editor();

		void Execute();
		void Update();

		void SetSelectedObjectIndex(int index) { mSelectedObjectIndex = index; }

		Camera* GetCamera() { return mCamera; }
		std::unique_ptr<Project>& GetActiveProject() { return mActiveProject; }
	private:
		void DrawObjectInspectorPanel(SceneObject* object);
	private:
		Camera* mCamera;

		std::unique_ptr<Project> mActiveProject;
		enum class ModalPurpose
		{
			NONE = -1, NEWPROJECT = 0
		};
		ModalPurpose mModalPurpose = ModalPurpose::NONE;
		bool mShowModal = false;
		bool mIsBlenderLinked = false;
		bool mShowHiearchy = false;
	private:
		int mSelectedObjectIndex = -1;
	};
}