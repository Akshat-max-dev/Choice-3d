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
		bool showMap = true;
		Texture2D* texture;
		~TextureMap();
	};

	struct Material
	{
		std::string Name;
		std::map<TEXTURE_MAP_TYPE, TextureMap*> TextureMaps;

		glm::vec4 Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
		float Roughness = 0.0f;
		float Metallic = 0.0f;
		~Material();
	};

	/*
	//Load Materials From Saved Files In Engine Format
	void LoadMaterials(std::ifstream& from, std::vector<Material*>& materials);

	//Load Materials Data
	bool LoadMaterialsData(std::ifstream& from, Texture2DData* data);

	//Save Materials Data
	void SaveMaterialsData(std::ofstream& to, Texture2DData* data);
	*/
}