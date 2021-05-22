#pragma once
#include "cpch.h"

#include "Texture.h"
#include <glm/glm.hpp>

namespace choice
{
	enum class TEXTURE_MAP_TYPE
	{
		ALBEDO = 0, NORMAL = 1, SPECULAR = 2, ROUGHNESS = 3, METALLIC = 4, AMBIENT_OCCLUSION = 5
	};

	struct TextureMap
	{
		std::string name;
		Texture2D* texture;
		~TextureMap();
	};

	struct Material
	{
		std::string Name;
		std::map<TEXTURE_MAP_TYPE, TextureMap*> TextureMaps;
		std::vector<char> Data;

		Material();
		~Material();
	};
}