#pragma once
#include "cpch.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace choice
{
	class Window
	{
	public:
		Window();
		~Window();

		GLFWwindow* GetWindow() { return mWindow; }
		const uint32_t& GetWidth()const { return mWidth; }
		const uint32_t& GetHeight()const { return mHeight; }

		void UpdateTitle(const char* title) { glfwSetWindowTitle(mWindow, title); }
	private:
		GLFWwindow* mWindow;
		uint32_t mWidth = 1600;
		uint32_t mHeight = 900;
	};
}