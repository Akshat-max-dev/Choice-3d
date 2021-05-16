#include "Mesh.h"

namespace choice
{
	Primitive::~Primitive()
	{
		if (vertexarray) { delete vertexarray; }
		if (material) { delete material; }
	}

	Mesh::~Mesh()
	{
		for (auto& primitive : primitives) { if (primitive) { delete primitive; } }
	}

	Mesh* Cube(const std::string name /*= "Cube"*/)
	{
		float vertices[] = {
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
			 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
			-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
			 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
			 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
			-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
		};

		Mesh* cube = new Mesh();

		cube->Name = name;
		cube->mesh_type = MESH_TYPE::CUBE;
		cube->node_data_type = NODE_DATA_TYPE::MESH;
		cube->primitives.push_back(new Primitive());
		cube->primitives[0]->vertexarray = new VertexArray();
		cube->primitives[0]->vertexarray->VertexBuffer(vertices, sizeof(vertices), "332");
		cube->primitives[0]->vertexarray->IndexBuffer(nullptr, 0);
		cube->primitives[0]->material = new Material();
		cube->boundingbox = CalculateBoundingBox(vertices, 36 * 8, 8);

		return cube;
	}

	Mesh* Sphere(const std::string name /*= "Sphere"*/)
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

		Mesh* sphere = new Mesh();

		sphere->Name = name;
		sphere->mesh_type = MESH_TYPE::SPHERE;
		sphere->node_data_type = NODE_DATA_TYPE::MESH;
		sphere->primitives.push_back(new Primitive());
		sphere->primitives[0]->vertexarray = new VertexArray();
		sphere->primitives[0]->vertexarray->VertexBuffer(vertices.data(), vertices.size() * sizeof(float), "332");
		sphere->primitives[0]->vertexarray->IndexBuffer(indices.data(), (uint32_t)indices.size());
		sphere->primitives[0]->material = new Material();
		sphere->boundingbox = CalculateBoundingBox(vertices.data(), (uint32_t)vertices.size(), 8);
		
		return sphere;
	}

}