#pragma once
#include "cpch.h"

#include "Project/Scene/Scene.h"
#include "Camera/EditorCamera.h"

namespace choice
{
	class Pipeline
	{
	public:
		virtual void Init(uint32_t w, uint32_t h) {}
		virtual void Visible(uint32_t w, uint32_t h) {}
		virtual void Update(Scene* scene, Camera* camera) {}
		virtual void Shutdown() {}

		void PickedObject(int pickedobject) { mPickedObjectId = pickedobject; }
		void MousePicking(bool value) { mMousePicking = value; }
	protected:
		int mPickedObjectId = -1;
		bool mMousePicking = true;
	};
}