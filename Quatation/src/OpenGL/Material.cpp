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
		glm::vec3* color = global::GlobalReflectionData.UniformBuffers["Material"]->MemberData<glm::vec3>("Material.Color", Data);
		*color = { 1.0f, 1.0f, 1.0f };
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

		uint32_t binding;

		switch (type)
		{
		case TEXTURE_MAP_TYPE::ALBEDO:			  binding = reflectiondata.Samplers["gAlbedoMap"]; break;
		case TEXTURE_MAP_TYPE::NORMAL:			  binding = reflectiondata.Samplers["gNormalMap"]; break;
		case TEXTURE_MAP_TYPE::ROUGHNESS:		  binding = reflectiondata.Samplers["gRoughnessMap"]; break;
		case TEXTURE_MAP_TYPE::METALLIC:		  binding = reflectiondata.Samplers["gMetallicMap"]; break;
		case TEXTURE_MAP_TYPE::AMBIENT_OCCLUSION: binding = reflectiondata.Samplers["gAoMap"]; break;
		}

		return binding;
	}

}