#pragma once
#include "cpch.h"

#include <glm/glm.hpp>
#include "OpenGL/VertexArray.h"

namespace choice
{
	struct BoundingBox
	{
		glm::vec3 Min;
		glm::vec3 Max;
	};

	BoundingBox CalculateBoundingBox(float* data, uint32_t size, uint32_t increment);
	std::vector<glm::vec3> ExpandAABB(BoundingBox bb);
#ifdef DEBUG
	VertexArray* CreateBoundingBox(std::vector<glm::vec3> aabb);
#endif
}