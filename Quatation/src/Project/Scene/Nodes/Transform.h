#pragma once
#include "cpch.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include<glm/gtx/quaternion.hpp>

namespace choice
{
	struct Transform
	{
		glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		glm::mat4 GetTransform()
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));
			return glm::translate(glm::mat4(1.0f), Position) * 
				rotation * glm::scale(glm::mat4(1.0f), Scale);
		}
	};

	bool DecomposeTransform(glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);
}