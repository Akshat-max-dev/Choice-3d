#include "Sphere.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace choice
{
	Sphere::Sphere()
	{
		mName = "Sphere";
		Setup();
	}

	void Sphere::Draw()
	{
		mMesh->Bind();
		uint32_t count = mMesh->GetCount();
		glDrawElements(GL_TRIANGLE_STRIP, count, GL_UNSIGNED_INT, nullptr);
	}

	void Sphere::Setup()
	{
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> texcoords;
		
		const uint32_t X_SEGMENTS = 64;
		const uint32_t Y_SEGMENTS = 64;
		const float PI = 3.14159265359f;

		for (uint32_t y = 0; y <= Y_SEGMENTS; ++y)
		{
			for (uint32_t x = 0; x <= X_SEGMENTS; ++x)
			{
				float xSegment = (float)x / (float)X_SEGMENTS;
				float ySegment = (float)y / (float)Y_SEGMENTS;
				float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
				float yPos = std::cos(ySegment * PI);
				float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

				positions.push_back({ xPos, yPos, zPos });
				normals.push_back({ xPos, yPos, zPos });
				texcoords.push_back({ xSegment, ySegment });
			}
		}

		std::vector<uint32_t> indices;

		bool oddRow = false;
		for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
		{
			if (!oddRow) // even rows: y == 0, y == 2; and so on
			{
				for (uint32_t x = 0; x <= X_SEGMENTS; ++x)
				{
					indices.push_back(y * (X_SEGMENTS + 1) + x);
					indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
				}
			}
			else
			{
				for (int x = X_SEGMENTS; x >= 0; --x)
				{
					indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
					indices.push_back(y * (X_SEGMENTS + 1) + x);
				}
			}
			oddRow = !oddRow;
		}
		
		std::vector<float> vertices;
		for (uint32_t i = 0; i < positions.size(); ++i)
		{
			vertices.push_back(positions[i].x);
			vertices.push_back(positions[i].y);
			vertices.push_back(positions[i].z);
			if (normals.size() > 0)
			{
				vertices.push_back(normals[i].x);
				vertices.push_back(normals[i].y);
				vertices.push_back(normals[i].z);
			}
			if (texcoords.size() > 0)
			{
				vertices.push_back(texcoords[i].x);
				vertices.push_back(texcoords[i].y);
			}
		}

		mMesh = new VertexArray();
		mMesh->VertexBuffer(vertices.data(), vertices.size() * sizeof(float), "332");
		mMesh->IndexBuffer(indices.data(), static_cast<uint32_t>(indices.size()));
	}

}