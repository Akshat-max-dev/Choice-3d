#pragma once
#include "cpch.h"

#include <glm/glm.hpp>

#include "Transform.h"
#include "BoundingBox.h"

namespace choice
{
	enum class LightType
	{
		NONE = -1, DIRECTIONAL = 0, POINT = 1
	};

	struct Light
	{
		std::string Name;
		LightType Type = LightType::NONE;
		glm::vec3 Color = glm::vec3(1.0f);
		float Intensity = 1.0f;
		float Radius = 1.0f;

		virtual std::vector<glm::mat4> ViewProjection(Transform* transform, BoundingBox* box) = 0;
	};

	struct DirectionalLight :public Light
	{
		std::vector<glm::mat4> ViewProjection(Transform* transform, BoundingBox* sceneaabb)override;
	};

	struct PointLight :public Light
	{
		std::vector<glm::mat4> ViewProjection(Transform* transform, BoundingBox* sceneaabb)override;
	};
}