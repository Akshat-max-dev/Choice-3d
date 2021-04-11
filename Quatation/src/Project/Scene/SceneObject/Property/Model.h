#pragma once
#include "cpch.h"

#include "OpenGL/VertexArray.h"
#include "OpenGL/Texture.h"
#include "Transform.h"

namespace choice
{
	struct DumpableMeshData
	{
		std::vector<float> Vertices;
		std::vector<uint32_t> Indices;
		uint32_t MaterialIndex;
	};

	struct DumpableTextureData
	{
		std::string Source;
		uint32_t magFilter, minFilter, wrapS, wrapT;
	};

	struct DumpableMaterialData
	{
		std::string MaterialName;
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
		~Model();
	};

	std::pair<Model*, Transform*> LoadModel(const std::string& srcFile);
	const std::string DumpModel(const std::string& srcFile, const std::string& dstDirectory);
}