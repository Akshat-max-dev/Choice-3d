#pragma once
#include "cpch.h"

#include <glm/glm.hpp>

#include "Node.h"
#include "BoundingBox.h"

namespace choice
{
	enum class LIGHT_TYPE
	{
		NONE = -1, DIRECTIONAL = 0, POINT = 1
	};

	struct Light :public Node
	{
		LIGHT_TYPE Type = LIGHT_TYPE::NONE;
		glm::vec3 Color = glm::vec3(1.0f);
		float Intensity = 1.0f;
		float Radius = 1.0f;

		virtual std::vector<glm::mat4> ViewProjection(BoundingBox* box) = 0;
	};

	struct DirectionalLightData
	{
		glm::vec3 Direction;
		glm::vec3 Color;
		glm::mat4 LightVP;
	};

	struct DirectionalLight :public Light
	{
		std::vector<glm::mat4> ViewProjection(BoundingBox* sceneaabb)override;
	};

	struct PointLightData
	{
		glm::vec3 Position;
		glm::vec3 Color;
		float Radius;
	};

	struct PointLight :public Light
	{
		std::vector<glm::mat4> ViewProjection(BoundingBox* sceneaabb)override;
	};
}