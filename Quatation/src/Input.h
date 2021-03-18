#pragma once
#ifndef INPUT_H_
#define INPUT_H_
#include "cpch.h"

#include <glm/glm.hpp>

#include "Choice.h"

namespace choice
{
	class Input
	{
	public:
		static bool IsKeyPressed(int key)
		{
			auto action = glfwGetKey(Choice::Instance()->GetWindow()->GetWindow(), key);
			return action == GLFW_PRESS;
		}

		static bool IsButtonPressed(int button)
		{
			auto action = glfwGetMouseButton(Choice::Instance()->GetWindow()->GetWindow(), button);
			return action == GLFW_PRESS;
		}

		static float GetMouseX() { return GetMousePosition().x; }
		static float GetMouseY() { return GetMousePosition().y; }
	private:
		static glm::vec2 GetMousePosition()
		{
			double xpos, ypos;
			glfwGetCursorPos(Choice::Instance()->GetWindow()->GetWindow(), &xpos, &ypos);
			return{ (float)xpos, (float)ypos };
		}
	};
}
#endif