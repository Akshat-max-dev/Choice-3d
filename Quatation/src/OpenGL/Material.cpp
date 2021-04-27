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
			from.read((char*)&material->Ao, sizeof(material->Ao));

			from.read((char*)&material->Color.r, sizeof(material->Color.r));
			from.read((char*)&material->Color.g, sizeof(material->Color.g));
			from.read((char*)&material->Color.b, sizeof(material->Color.b));
			from.read((char*)&material->Color.a, sizeof(material->Color.a));

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
		uint32_t sourcefilenamesize;
		from.read((char*)&sourcefilenamesize, sizeof(sourcefilenamesize));
		if (sourcefilenamesize)
		{
			data->Source.resize(sourcefilenamesize);
			from.read((char*)data->Source.data(), sourcefilenamesize);

			from.read((char*)&data->magFilter, sizeof(data->magFilter));
			from.read((char*)&data->minFilter, sizeof(data->minFilter));
			from.read((char*)&data->wrapS, sizeof(data->wrapS));
			from.read((char*)&data->wrapT, sizeof(data->wrapT));

			return true;
		}
		return false;
	}

	void SaveMaterialsData(std::ofstream& to, Texture2DData* data)
	{
		if (data)
		{
			uint32_t sourcefilenamesize = (uint32_t)data->Source.size();
			to.write((char*)&sourcefilenamesize, sizeof(sourcefilenamesize));
			to.write((char*)data->Source.data(), sourcefilenamesize);

			to.write((char*)&data->magFilter, sizeof(data->magFilter));
			to.write((char*)&data->minFilter, sizeof(data->minFilter));
			to.write((char*)&data->wrapS, sizeof(data->wrapS));
			to.write((char*)&data->wrapT, sizeof(data->wrapT));
		}
		else
		{
			uint32_t sourcefilenamesize = 0;
			to.write((char*)&sourcefilenamesize, sizeof(sourcefilenamesize));
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