#pragma once
#include "cpch.h"

#include "OpenGL/VertexArray.h"
#include "OpenGL/Material.h"
#include "BoundingBox.h"
#include "Node.h"

namespace choice
{
	struct Primitive
	{
		VertexArray* vertexarray;
		Material* material;
		~Primitive();
	};

	enum class MESH_TYPE
	{
		NONE = -1, CUBE = 0, SPHERE = 1, IMPORTED = 2
	};

	struct Mesh :public Node
	{
		std::vector<Primitive*> primitives;
		BoundingBox boundingbox;
		MESH_TYPE mesh_type;

		Mesh();
		~Mesh();
	};

	Mesh* Cube(const std::string name = "Cube");
	Mesh* Sphere(const std::string name = "Sphere");
}