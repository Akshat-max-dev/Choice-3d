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
		std::vector<char> Data;
		virtual std::vector<glm::mat4> ViewProjection(BoundingBox* box) = 0;

		virtual ~Light();
	};

	struct DirectionalLight :public Light
	{
		DirectionalLight();
		std::vector<glm::mat4> ViewProjection(BoundingBox* sceneaabb)override;
	};

	struct PointLight :public Light
	{
		PointLight();
		std::vector<glm::mat4> ViewProjection(BoundingBox* sceneaabb)override;
	};

	Light* CreateDiretionalLight(const std::string& name = "Directional Light");
	Light* CreatePointLight(const std::string& name = "Point Light");
}