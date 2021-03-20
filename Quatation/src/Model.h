#pragma once
#include "cpch.h"

#include "OpenGL/VertexArray.h"

namespace choice
{
	struct Model
	{
		std::string Name;
		std::vector<std::pair<VertexArray*, uint32_t>> Meshes;

		~Model();
	};

	Model* LoadModel(const std::string& srcFile);
}