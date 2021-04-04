#pragma once
#include "cpch.h"

#include "OpenGL/VertexArray.h"
#include "OpenGL/Texture.h"

#include <glm/glm.hpp>

namespace choice
{
	struct DumpableMeshData
	{
		std::vector<float> Vertices;
		std::vector<uint32_t> Indices;
	};

	struct DumpableTextureData
	{
		std::string Source;
		uint32_t magFilter, minFilter, wrapS, wrapT;
	};

	struct DumpableMaterialData
	{
		DumpableTextureData DiffuseMap;
		DumpableTextureData NormalMap;
	};

	struct Material
	{
		Texture2D* DiffuseMap;
		Texture2D* NormalMap;
		~Material();
	};

	struct Model
	{
		std::string Name;
		std::vector<Material*> Materials;
		std::vector<std::pair<VertexArray*, uint32_t>> Meshes;
		glm::mat4 Transform = glm::mat4(1.0f);
		~Model();
	};

	Model* LoadModel(const std::string& srcFile);
	const std::string DumpModel(const std::string& srcFile, const std::string& dstDirectory);
}