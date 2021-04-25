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

			uint32_t namesize;
			from.read((char*)&namesize, sizeof(namesize));
			material->Name.resize(namesize);
			from.read((char*)material->Name.data(), namesize);

			from.read((char*)&material->Roughness, sizeof(material->Roughness));
			from.read((char*)&material->Metallic, sizeof(material->Metallic));

			from.read((char*)&material->Color.r, sizeof(material->Color.r));
			from.read((char*)&material->Color.g, sizeof(material->Color.g));
			from.read((char*)&material->Color.b, sizeof(material->Color.b));
			from.read((char*)&material->Color.a, sizeof(material->Color.a));

			Texture2DData* diffusemapdata = new Texture2DData();

			uint32_t diffusemapnamesize;
			from.read((char*)&diffusemapnamesize, sizeof(diffusemapnamesize));
			if (diffusemapnamesize)
			{
				diffusemapdata->Source.resize(diffusemapnamesize);
				from.read((char*)diffusemapdata->Source.data(), diffusemapnamesize);

				from.read((char*)&diffusemapdata->magFilter, sizeof(diffusemapdata->magFilter));
				from.read((char*)&diffusemapdata->minFilter, sizeof(diffusemapdata->minFilter));
				from.read((char*)&diffusemapdata->wrapS, sizeof(diffusemapdata->wrapS));
				from.read((char*)&diffusemapdata->wrapT, sizeof(diffusemapdata->wrapT));
			}

			if (!diffusemapdata->Source.empty())
			{
				material->DiffuseMap.first = true;
				material->DiffuseMap.second.first = new Texture2D(LoadTexture2D(*diffusemapdata));
				material->DiffuseMap.second.second = diffusemapdata;
			}
			else
			{
				delete diffusemapdata;
			}

			Texture2DData* normalmapdata = new Texture2DData();

			uint32_t normalmapnamesize;
			from.read((char*)&normalmapnamesize, sizeof(normalmapnamesize));
			if (normalmapnamesize)
			{
				normalmapdata->Source.resize(normalmapnamesize);
				from.read((char*)normalmapdata->Source.data(), normalmapnamesize);

				from.read((char*)&normalmapdata->magFilter, sizeof(normalmapdata->magFilter));
				from.read((char*)&normalmapdata->minFilter, sizeof(normalmapdata->minFilter));
				from.read((char*)&normalmapdata->wrapS, sizeof(normalmapdata->wrapS));
				from.read((char*)&normalmapdata->wrapT, sizeof(normalmapdata->wrapT));
			}

			if (!normalmapdata->Source.empty())
			{
				material->NormalMap.first = true;
				material->NormalMap.second.first = new Texture2D(LoadTexture2D(*normalmapdata));
				material->NormalMap.second.second = normalmapdata;
			}
			else
			{
				delete normalmapdata;
			}
		}
	}

	Material::~Material()
	{
		if (DiffuseMap.second.first)delete DiffuseMap.second.first;
		if (NormalMap.second.first)delete NormalMap.second.first;
		if (DiffuseMap.second.second)delete DiffuseMap.second.second;
		if (NormalMap.second.second)delete NormalMap.second.second;
	}

}