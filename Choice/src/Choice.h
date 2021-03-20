#pragma once
#include "cpch.h"

#include "Window.h"
#include "Camera/EditorCamera.h"
#include "OpenGL/Shader.h"
#include "OpenGL/VertexArray.h"
#include "Model.h"

namespace choice
{
	class Choice
	{
	public:
		Choice();
		~Choice();

		void run();

		std::unique_ptr<Window>& GetWindow() { return mWindow; }
		//Temp
		std::unique_ptr<Camera>& GetCamera() { return mCamera; }

		static Choice* Instance() { return sInstance; }
	private:
		static Choice* sInstance;
	private:
		std::unique_ptr<Window> mWindow;
		std::unique_ptr<Camera> mCamera;
		//Temp
		std::unique_ptr<Shader> mShader;
		Model* mModel;
	};
}