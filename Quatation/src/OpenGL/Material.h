#pragma once
#include "cpch.h"

#include "Texture.h"

namespace choice
{
	struct Material
	{
		std::pair<Texture2D*, Texture2DData*> DiffuseMap;
		std::pair<Texture2D*, Texture2DData*> NormalMap;
		float Roughness = 0.0f;
		float Metallic = 0.0f;
		~Material();
	};

	//Load Materials From Saved Files In Engine Format
	void LoadMaterials(std::ifstream& from, std::vector<Material*>& materials);
}