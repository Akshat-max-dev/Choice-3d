#pragma once
#include "cpch.h"

#include "Camera/Camera.h"
#include "Camera/EditorCamera.h"

#include "OpenGL/Shader.h"
#include "Model.h"

namespace choice
{
	class Editor
	{
	public:
		Editor(uint32_t w, uint32_t h);
		~Editor();

		void Execute();
		void Update();

		std::unique_ptr<Camera>& GetCamera() { return mCamera; }
	private:
		std::unique_ptr<Camera> mCamera;

		//Temp
		std::unique_ptr<Shader> mShader;
		Model* mModel;
	};
}