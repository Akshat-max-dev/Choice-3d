#include "BoundingBox.h"

namespace choice
{
	BoundingBox CalculateBoundingBox(float* data, uint32_t size, uint32_t increment)
	{
		float minX = std::numeric_limits<float>::max();
		float minY = minX;
		float minZ = minY;
		float maxX = std::numeric_limits<float>::min();
		float maxY = maxX;
		float maxZ = maxY;
		
		if (data)
		{
			for (uint32_t i = 0; i < size; i += increment)
			{
				minX = data[i] < minX ? data[i] : minX;
				minY = data[i + 1] < minY ? data[i + 1] : minY;
				minZ = data[i + 2] < minZ ? data[i + 2] : minZ;

				maxX = data[i] > maxX ? data[i] : maxX;
				maxY = data[i + 1] > maxY ? data[i + 1] : maxY;
				maxZ = data[i + 2] > maxZ ? data[i + 2] : maxZ;
			}
		}

		BoundingBox bb = {};
		bb.Min = { minX, minY, minZ };
		bb.Max = { maxX, maxY, maxZ };
		return bb;
	}

	std::vector<glm::vec3> ExpandAABB(BoundingBox bb)
	{
		std::vector<glm::vec3> b(8);
		b[0] = { bb.Min.x, bb.Min.y, bb.Min.z };
		b[1] = { bb.Max.x, bb.Min.y, bb.Min.z };
		b[2] = { bb.Max.x, bb.Max.y, bb.Min.z };
		b[3] = { bb.Min.x, bb.Max.y, bb.Min.z };
		b[4] = { bb.Min.x, bb.Min.y, bb.Max.z };
		b[5] = { bb.Max.x, bb.Min.y, bb.Max.z };
		b[6] = { bb.Max.x, bb.Max.y, bb.Max.z };
		b[7] = { bb.Min.x, bb.Max.y, bb.Max.z };
		return b;
	}

#ifdef DEBUG
	VertexArray* CreateBoundingBox(std::vector<glm::vec3> aabb)
	{
		std::vector<float> vertices;

		for (uint32_t i = 0; i < aabb.size(); i++)
		{
			vertices.push_back(aabb[i].x);
			vertices.push_back(aabb[i].y);
			vertices.push_back(aabb[i].z);
		}

		uint32_t indices[] = {
			0, 1,
			2, 3,
			4, 5,
			6, 7,
			0, 4,
			1, 5,
			2, 6,
			3, 7,
			5, 6,
			4, 7,
			0, 3,
			1, 2
		};

		VertexArray* va = new VertexArray();
		va->VertexBuffer(vertices.data(), vertices.size() * sizeof(float), "3");
		va->IndexBuffer(indices, sizeof(indices) / sizeof(uint32_t));

		return va;
	}
#endif
}