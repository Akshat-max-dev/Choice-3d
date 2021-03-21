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
		Choice::Instance()->GetEditor()->GetCamera()->OnMove(xpos, ypos);
	}

	void scrollcallback(GLFWwindow* window, double xoffset, double yoffset)
	{
		Choice::Instance()->GetEditor()->GetCamera()->OnScroll(yoffset);
	}

	void mousebuttoncallback(GLFWwindow* window, int button, int action, int mods)
	{
		switch (action)
		{
		case GLFW_PRESS:
			Choice::Instance()->GetEditor()->GetCamera()->OnButtonDown(button);
			break;
		case GLFW_RELEASE:
			Choice::Instance()->GetEditor()->GetCamera()->OnButtonUp(button);
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
		mGUI = std::make_unique<GUI>();
		mEditor = std::make_unique<Editor>(mWindow->GetWidth(), mWindow->GetHeight());	
	}

	Choice::~Choice()
	{
	}

	void Choice::run()
	{
		while (!glfwWindowShouldClose(mWindow->GetWindow()))
		{
			glClear(GL_COLOR_BUFFER_BIT);
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			
			mEditor->Update();

			mGUI->Begin();
			mEditor->Execute();
			mGUI->End();

			glfwSwapBuffers(mWindow->GetWindow());
			glfwPollEvents();
		}
	}

}