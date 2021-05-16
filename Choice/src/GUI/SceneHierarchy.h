#pragma once
#include "cpch.h"

#include "Project/Scene/Scene.h"

namespace choice
{
	class SceneHierarchy
	{
	public:
		SceneHierarchy();
		~SceneHierarchy();

		Node* SelectedNode() { return mSelectedNode; }
		void Execute(Scene* scene);
	private:
		void IterateNode(Node* node);
	private:
		Node* mSelectedNode = {};
	};

	void ShowAddingMenu(Scene* scene);
}