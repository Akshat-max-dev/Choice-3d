#pragma once
#include "cpch.h"

#include "Texture.h"
#include <glm/glm.hpp>

namespace choice
{
	struct Material
	{
		std::string Name;
		std::pair<bool, std::pair<Texture2D*, Texture2DData*>> DiffuseMap;
		std::pair<bool, std::pair<Texture2D*, Texture2DData*>> NormalMap;
		std::pair<bool, std::pair<Texture2D*, Texture2DData*>> RoughnessMap;
		std::pair<bool, std::pair<Texture2D*, Texture2DData*>> MetallicMap;
		std::pair<bool, std::pair<Texture2D*, Texture2DData*>> AOMap;
		float Roughness = 0.0f;
		float Metallic = 0.0f;
		float Ao = 1.0f;
		glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		~Material();
	};

	//Load Materials From Saved Files In Engine Format
	void LoadMaterials(std::ifstream& from, std::vector<Material*>& materials);

	//Load Materials Data
	bool LoadMaterialsData(std::ifstream& from, Texture2DData* data);

	//Save Materials Data
	void SaveMaterialsData(std::ofstream& to, Texture2DData* data);
}