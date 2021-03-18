#include "Window.h"

namespace choice
{
	Window::Window()
	{
		choiceassert(glfwInit());
		mWindow = glfwCreateWindow(mWidth, mHeight, "Choice", 0, 0);
		choiceassert(mWindow);

		glfwMakeContextCurrent(mWindow);
		glfwMaximizeWindow(mWindow);

		choiceassert(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress));
		choiceassert((GLVersion.minor >= 5 && GLVersion.major == 4));
	}

	Window::~Window()
	{
		glfwTerminate();
	}
}