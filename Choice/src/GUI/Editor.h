#pragma once
#include "cpch.h"

#include "Camera/Camera.h"
#include "Camera/EditorCamera.h"

#include "Project/Project.h"

#include "SceneHierarchy.h"
#include "NodeInspector.h"
#include "ProjectExplorer.h"

namespace choice
{
	namespace global
	{
		inline std::string ActiveSceneDir;
		inline std::string ActiveProjectDir;
		inline std::string ActiveProjectName;
		inline std::string ActiveSceneName;
	}

	class Editor
	{
	public:
		Editor(uint32_t w, uint32_t h);
		~Editor();

		void Execute();
		void Update();

		Camera* GetCamera() { return mCamera; }
		Project* GetActiveProject() { return mActiveProject; }
	private:
		void ShowFileMenu();
	private:
		Camera* mCamera;
		Project* mActiveProject;

		SceneHierarchy* mSceneHierarchy;
		NodeInspector* mNodeInspector;
		ProjectExplorer* mProjectExplorer;

		enum class ModalPurpose
		{
			NONE = -1, NEWPROJECT = 0
		};
		ModalPurpose mModalPurpose = ModalPurpose::NONE;
		bool mShowModal = false;
		bool mIsBlenderLinked = false;
	private:
		int mGizmoType = -1;
		glm::vec2 mVisibleRegion;
	};
}