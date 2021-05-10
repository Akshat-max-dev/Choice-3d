#include "Material.h"

#include "BinaryHelper.h"

namespace choice
{
	void LoadMaterials(std::ifstream& from, std::vector<Material*>& materials)
	{
		uint32_t materialssize;
		Binary::Read<uint32_t>(from, materialssize);
		materials.resize(materialssize);
		for (auto& material : materials)
		{
			material = new Material();

			Binary::Read<std::string>(from, material->Name);

			Binary::Read<float>(from, material->Roughness);
			Binary::Read<float>(from, material->Metallic);
			Binary::Read<float>(from, material->Ao);

			Binary::Read<glm::vec4>(from, material->Color);

			//Diffuse Map
			Texture2DData* diffusemapdata = new Texture2DData();

			if (LoadMaterialsData(from, diffusemapdata))
			{
				material->DiffuseMap.first = true;
				material->DiffuseMap.second.first = new Texture2D(LoadTexture2D(*diffusemapdata));
				material->DiffuseMap.second.second = diffusemapdata;
			}
			else { delete diffusemapdata; }

			//Normal Map
			Texture2DData* normalmapdata = new Texture2DData();

			if (LoadMaterialsData(from, normalmapdata))
			{
				material->NormalMap.first = true;
				material->NormalMap.second.first = new Texture2D(LoadTexture2D(*normalmapdata));
				material->NormalMap.second.second = normalmapdata;
			}
			else { delete normalmapdata; }

			//Roughness Map
			Texture2DData* roughnessmapdata = new Texture2DData();

			if (LoadMaterialsData(from, roughnessmapdata))
			{
				material->RoughnessMap.first = true;
				material->RoughnessMap.second.first = new Texture2D(LoadTexture2D(*roughnessmapdata));
				material->RoughnessMap.second.second = roughnessmapdata;
			}
			else { delete roughnessmapdata; }

			//Metallic Map
			Texture2DData* metallicmapdata = new Texture2DData();

			if (LoadMaterialsData(from, metallicmapdata))
			{
				material->MetallicMap.first = true;
				material->MetallicMap.second.first = new Texture2D(LoadTexture2D(*metallicmapdata));
				material->MetallicMap.second.second = metallicmapdata;
			}
			else { delete metallicmapdata; }

			//Ambient Occlusion Map
			Texture2DData* aomapdata = new Texture2DData();

			if (LoadMaterialsData(from, aomapdata))
			{
				material->AOMap.first = true;
				material->AOMap.second.first = new Texture2D(LoadTexture2D(*aomapdata));
				material->AOMap.second.second = aomapdata;
			}
			else { delete aomapdata; }
		}
	}

	bool LoadMaterialsData(std::ifstream& from, Texture2DData* data)
	{
		Binary::Read<std::string>(from, data->Source);
		if (!data->Source.empty())
		{
			Binary::Read(from, data->magFilter);
			Binary::Read(from, data->minFilter);
			Binary::Read(from, data->wrapS);
			Binary::Read(from, data->wrapT);

			return true;
		}
		return false;
	}

	void SaveMaterialsData(std::ofstream& to, Texture2DData* data)
	{
		if (data)
		{
			Binary::Write<std::string>(to, data->Source);

			Binary::Write(to, data->magFilter);
			Binary::Write(to, data->minFilter);
			Binary::Write(to, data->wrapS);
			Binary::Write(to, data->wrapT);
		}
		else
		{
			uint32_t sourcefilenamesize = 0;
			Binary::Write<uint32_t>(to, sourcefilenamesize);
		}
	}

	Material::~Material()
	{
		if (DiffuseMap.second.first)   delete DiffuseMap.second.first;
		if (DiffuseMap.second.second)  delete DiffuseMap.second.second;
		if (NormalMap.second.first)    delete NormalMap.second.first;
		if (NormalMap.second.second)   delete NormalMap.second.second;
		if (RoughnessMap.second.first) delete RoughnessMap.second.first;
		if (RoughnessMap.second.second)delete RoughnessMap.second.second;
		if (MetallicMap.second.first)  delete MetallicMap.second.first;
		if (MetallicMap.second.second) delete MetallicMap.second.second;
		if (AOMap.second.first)		   delete AOMap.second.first;
		if (AOMap.second.second)	   delete AOMap.second.second;
	}

}