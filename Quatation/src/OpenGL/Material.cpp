#include "Material.h"

#include "BinaryHelper.h"

namespace choice
{
	Material::Material()
	{
		Name = "Material 0";
		TextureMaps.insert({ TEXTURE_MAP_TYPE::ALBEDO , new TextureMap() });
	}

	Material::~Material()
	{
		for (auto& texturemap : TextureMaps)
		{
			if (texturemap.second) { delete texturemap.second; }
		}
	}

	TextureMap::~TextureMap()
	{
		if (texture) { delete texture; }
	}

}