#include "Material.h"

#include "ReflectionData.h"

namespace choice
{
	Material::Material()
	{
		Name = "Material 0";
		TextureMaps.insert({ TEXTURE_MAP_TYPE::ALBEDO , new TextureMap() });
		Data.resize(global::GlobalReflectionData.UniformBuffers["Material"]->GetBufferSize());

		//Set Default Material Color As White
		glm::vec4* color = global::GlobalReflectionData.UniformBuffers["Material"]->MemberData<glm::vec4>("Material.Color", Data);
		*color = { 1.0f, 1.0f, 1.0f, 1.0f };
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

	const uint32_t GetBinding(TEXTURE_MAP_TYPE type)
	{
		ReflectionData& reflectiondata = global::GlobalReflectionData;

		switch (type)
		{
		case TEXTURE_MAP_TYPE::ALBEDO:			  return reflectiondata.Samplers["gAlbedoMap"];
		case TEXTURE_MAP_TYPE::NORMAL:			  return reflectiondata.Samplers["gNormalMap"];
		case TEXTURE_MAP_TYPE::ROUGHNESS:		  return reflectiondata.Samplers["gRoughnessMap"];
		case TEXTURE_MAP_TYPE::METALLIC:		  return reflectiondata.Samplers["gMetallicMap"];
		case TEXTURE_MAP_TYPE::AMBIENT_OCCLUSION: return reflectiondata.Samplers["gAoMap"];
		}
	}

}