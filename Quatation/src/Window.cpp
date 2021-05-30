#include "Window.h"

#include "Error.h"

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

		Message<INFO>("------------------------------------------------", MESSAGE_ORIGIN::NONE);
		Message<INFO>((const char*)glGetString(GL_VENDOR), MESSAGE_ORIGIN::OPENGL);
		Message<INFO>((const char*)glGetString(GL_RENDERER), MESSAGE_ORIGIN::OPENGL);
		Message<INFO>((const char*)glGetString(GL_VERSION), MESSAGE_ORIGIN::OPENGL);
		Message<INFO>("------------------------------------------------", MESSAGE_ORIGIN::NONE);

		choiceassert((GLVersion.minor >= 5 && GLVersion.major == 4));
	}

	Window::~Window()
	{
		glfwTerminate();
	}
}