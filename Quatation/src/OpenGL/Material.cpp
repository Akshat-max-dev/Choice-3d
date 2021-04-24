#include "Material.h"

namespace choice
{
	void LoadMaterials(std::ifstream& from, std::vector<Material*>& materials)
	{
		uint32_t materialssize;
		from.read((char*)&materialssize, sizeof(materialssize));
		materials.resize(materialssize);
		for (auto& material : materials)
		{
			material = new Material();

			from.read((char*)&material->Roughness, sizeof(material->Roughness));
			from.read((char*)&material->Metallic, sizeof(material->Metallic));

			Texture2DData* diffusemapdata = new Texture2DData();

			uint32_t diffusemapnamesize;
			from.read((char*)&diffusemapnamesize, sizeof(diffusemapnamesize));
			diffusemapdata->Source.resize(diffusemapnamesize);
			from.read((char*)diffusemapdata->Source.data(), diffusemapnamesize);

			from.read((char*)&diffusemapdata->magFilter, sizeof(diffusemapdata->magFilter));
			from.read((char*)&diffusemapdata->minFilter, sizeof(diffusemapdata->minFilter));
			from.read((char*)&diffusemapdata->wrapS, sizeof(diffusemapdata->wrapS));
			from.read((char*)&diffusemapdata->wrapT, sizeof(diffusemapdata->wrapT));

			if (!diffusemapdata->Source.empty())
			{
				material->DiffuseMap.first = new Texture2D(LoadTexture2D(*diffusemapdata));
				material->DiffuseMap.second = diffusemapdata;
			}
			else
			{
				delete diffusemapdata;
			}

			Texture2DData* normalmapdata = new Texture2DData();

			uint32_t normalmapnamesize;
			from.read((char*)&normalmapnamesize, sizeof(normalmapnamesize));
			normalmapdata->Source.resize(normalmapnamesize);
			from.read((char*)normalmapdata->Source.data(), normalmapnamesize);

			from.read((char*)&normalmapdata->magFilter, sizeof(normalmapdata->magFilter));
			from.read((char*)&normalmapdata->minFilter, sizeof(normalmapdata->minFilter));
			from.read((char*)&normalmapdata->wrapS, sizeof(normalmapdata->wrapS));
			from.read((char*)&normalmapdata->wrapT, sizeof(normalmapdata->wrapT));

			if (!normalmapdata->Source.empty())
			{
				material->NormalMap.first = new Texture2D(LoadTexture2D(*normalmapdata));
				material->NormalMap.second = normalmapdata;
			}
			else
			{
				delete normalmapdata;
			}
		}
	}

	Material::~Material()
	{
		if (DiffuseMap.first)delete DiffuseMap.first;
		if (NormalMap.first)delete NormalMap.first;
		if (DiffuseMap.second)delete DiffuseMap.second;
		if (NormalMap.second)delete NormalMap.second;
	}

}