#include "Choice.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Input.h"

namespace choice
{
	void windowclosecallback(GLFWwindow* window)
	{
		glfwSetWindowShouldClose(window, 1);
	}

	void windowsizecallback(GLFWwindow* window, int width, int height)
	{
		//To do
	}

	void cursorposcallback(GLFWwindow* window, double xpos, double ypos)
	{
		Choice::Instance()->GetCamera()->OnMove(xpos, ypos);
	}

	void scrollcallback(GLFWwindow* window, double xoffset, double yoffset)
	{
		Choice::Instance()->GetCamera()->OnScroll(yoffset);
	}

	void mousebuttoncallback(GLFWwindow* window, int button, int action, int mods)
	{
		switch (action)
		{
		case GLFW_PRESS:
			Choice::Instance()->GetCamera()->OnButtonDown(button);
			break;
		case GLFW_RELEASE:
			Choice::Instance()->GetCamera()->OnButtonUp(button);
			break;
		}
	}

	void InputCallbacks(GLFWwindow* window)
	{
		glfwSetWindowCloseCallback(window, windowclosecallback);
		glfwSetWindowSizeCallback(window, windowsizecallback);
		glfwSetCursorPosCallback(window, cursorposcallback);
		glfwSetScrollCallback(window, scrollcallback);
		glfwSetMouseButtonCallback(window, mousebuttoncallback);
	}

	Choice* Choice::sInstance = nullptr;
	Choice::Choice()
	{
		sInstance = this;

		mWindow = std::make_unique<Window>();

		InputCallbacks(mWindow->GetWindow());

		mCamera = std::make_unique<EditorCamera>((float)mWindow->GetWidth()/(float)mWindow->GetHeight());
		
		mShader = std::make_unique<Shader>("Choice/assets/shaders/Test.glsl");
		mModel = LoadModel("E:/untitled.glb");
	}

	Choice::~Choice()
	{
		delete mModel;
	}

	void Choice::run()
	{
		while (!glfwWindowShouldClose(mWindow->GetWindow()))
		{
			glClear(GL_COLOR_BUFFER_BIT);
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
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
			glfwSwapBuffers(mWindow->GetWindow());
			glfwPollEvents();
		}
	}

}