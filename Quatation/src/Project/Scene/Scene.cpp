#include "Scene.h"

#include <glad/glad.h>

namespace choice
{
	Scene::Scene(const std::string& name, const std::string& directory)
		:mName(name), mDirectory(directory)
	{
		ghc::filesystem::create_directory(mDirectory + "\\" + mName);
		ghc::filesystem::create_directory(mDirectory + "\\" + mName + "\\" + "Assets");

		ghc::filesystem::copy("Choice/assets/hdri_skybox/crosswalk_4k.hdr", mDirectory + "\\" + mName + "\\" + "Assets\\crosswalk_4k.hdr");

		SceneObject* object = new SceneObject();
		std::string dstHDRI = mDirectory + "\\" + mName + "\\" + "Assets\\crosswalk_4k.hdr";
		object->AddProperty<Skybox>(new Skybox(dstHDRI));
		mSceneObjects.push_back(object);

		SceneObject* _object = new SceneObject();
		_object->AddProperty<Light>(new DirectionalLight());
		Transform* transform = new Transform();
		transform->Position = { 0.0f, 0.0f, 0.0f };
		transform->Rotation = { 0.0f, 0.0f, 0.0f };
		transform->Scale = { 1.0f, 1.0f, 1.0f };
		_object->AddProperty<Transform>(transform);
		mSceneObjects.push_back(_object);
	}

	Scene::Scene(const std::string& cscene)
	{
		std::string temp = cscene.substr(cscene.find_last_of('\\') + 1, cscene.size());
		mName = temp.substr(0, temp.find_last_of('.'));
		temp = cscene.substr(0, cscene.find_last_of('\\'));
		mDirectory = temp.substr(0, temp.size() - mName.size() - 1);

		std::ifstream containedscene(cscene, std::ios::in | std::ios::binary);
		if (containedscene.bad() && !containedscene.is_open())
		{
			std::cout << "Failed To Load Scene" << std::endl;
			return;
		}

		uint32_t sceneobjectssize;
		containedscene.read((char*)&sceneobjectssize, sizeof(sceneobjectssize));
		mSceneObjects.resize(sceneobjectssize);
		for (auto& object : mSceneObjects)
		{
			object = new SceneObject();

			uint32_t skyboxsrcFileSize;
			containedscene.read((char*)&skyboxsrcFileSize, sizeof(skyboxsrcFileSize));
			if (skyboxsrcFileSize)
			{
				std::string skyboxsrcFile;
				skyboxsrcFile.resize(skyboxsrcFileSize);
				containedscene.read((char*)skyboxsrcFile.data(), skyboxsrcFileSize);

				object->AddProperty<Skybox>(new Skybox(skyboxsrcFile));
			}
			
			uint32_t modelsrcFilesize;
			containedscene.read((char*)&modelsrcFilesize, sizeof(modelsrcFilesize));
			if (modelsrcFilesize)
			{
				std::string modelsrcFile;
				modelsrcFile.resize(modelsrcFilesize);
				containedscene.read((char*)modelsrcFile.data(), modelsrcFilesize);

				object->AddProperty<Model>(LoadModel(modelsrcFile));
			}
			
			int lightype;
			containedscene.read((char*)&lightype, sizeof(lightype));
			if (lightype != -1)
			{
				uint32_t lightnamesize;
				containedscene.read((char*)&lightnamesize, sizeof(lightnamesize));
				std::string lightname;
				lightname.resize(lightnamesize);
				containedscene.read((char*)lightname.data(), lightnamesize);

				glm::vec3 color;
				containedscene.read((char*)&color.x, sizeof(color.x));
				containedscene.read((char*)&color.y, sizeof(color.y));
				containedscene.read((char*)&color.z, sizeof(color.z));

				float intensity;
				containedscene.read((char*)&intensity, sizeof(intensity));

				float radius;
				containedscene.read((char*)&radius, sizeof(radius));

				switch (lightype)
				{
				case 0:
					object->AddProperty<Light>(new DirectionalLight(lightname, color, intensity));
					break;
				case 1:
					object->AddProperty<Light>(new PointLight(lightname, color, intensity, radius));
					break;
				}
			}

			if (modelsrcFilesize || lightype != -1)
			{
				Transform* transform = new Transform();

				containedscene.read((char*)&transform->Position.x, sizeof(transform->Position.x));
				containedscene.read((char*)&transform->Position.y, sizeof(transform->Position.y));
				containedscene.read((char*)&transform->Position.z, sizeof(transform->Position.z));

				containedscene.read((char*)&transform->Rotation.x, sizeof(transform->Rotation.x));
				containedscene.read((char*)&transform->Rotation.y, sizeof(transform->Rotation.y));
				containedscene.read((char*)&transform->Rotation.z, sizeof(transform->Rotation.z));

				containedscene.read((char*)&transform->Scale.x, sizeof(transform->Scale.x));
				containedscene.read((char*)&transform->Scale.y, sizeof(transform->Scale.y));
				containedscene.read((char*)&transform->Scale.z, sizeof(transform->Scale.z));

				object->AddProperty<Transform>(transform);
			}
		}
		containedscene.close();
	}

	Scene::~Scene()
	{
		for (auto& object : mSceneObjects)
		{
			if (object) { delete object; }
		}
	}

	void Scene::Save()
	{
		std::ofstream cscene(mDirectory + "\\" + mName + "\\" + mName + ".cscene", std::ios::out | std::ios::binary);
		if (cscene.fail())
		{
			std::cout << "Cannot Save Scene" << std::endl;
			return;
		}

		uint32_t sceneobjectssize = (uint32_t)mSceneObjects.size();
		cscene.write((char*)&sceneobjectssize, sizeof(sceneobjectssize));
		for (auto& object : mSceneObjects)
		{
			if (object)
			{
				auto skyboxprop = object->GetProperty<Skybox>();
				if (skyboxprop)
				{
					uint32_t skyboxsrcFilesize = (uint32_t)skyboxprop->GetFilepath().size();
					cscene.write((char*)&skyboxsrcFilesize, sizeof(skyboxsrcFilesize));
					cscene.write(skyboxprop->GetFilepath().data(), skyboxsrcFilesize);
				}
				else
				{
					uint32_t skyboxsrcFilesize = 0;
					cscene.write((char*)&skyboxsrcFilesize, sizeof(skyboxsrcFilesize));
				}

				auto modelprop = object->GetProperty<Model>();
				if (modelprop)
				{
					std::string modelsrcFile = mDirectory + "\\" + mName + "\\" + "Assets\\" + modelprop->Name + ".cmodel";
					uint32_t modelsrcFileSize = (uint32_t)modelsrcFile.size();
					cscene.write((char*)&modelsrcFileSize, sizeof(modelsrcFileSize));
					cscene.write(modelsrcFile.data(), modelsrcFileSize);
				}
				else
				{
					uint32_t modelsrcFileSize = 0;
					cscene.write((char*)&modelsrcFileSize, sizeof(modelsrcFileSize));
				}

				auto lightprop = object->GetProperty<Light>();
				if (lightprop)
				{
					int lighttype = static_cast<int>(lightprop->GetLightType());
					cscene.write((char*)&lighttype, sizeof(lighttype));

					uint32_t lightnamesize = (uint32_t)lightprop->GetName().size();
					cscene.write((char*)&lightnamesize, sizeof(lightnamesize));
					cscene.write((char*)lightprop->GetName().data(), lightnamesize);

					cscene.write((char*)&lightprop->GetDiffuse().x, sizeof(lightprop->GetDiffuse().x));
					cscene.write((char*)&lightprop->GetDiffuse().y, sizeof(lightprop->GetDiffuse().y));
					cscene.write((char*)&lightprop->GetDiffuse().z, sizeof(lightprop->GetDiffuse().z));

					cscene.write((char*)&lightprop->GetIntensity(), sizeof(lightprop->GetIntensity()));
					cscene.write((char*)&lightprop->GetRadius(), sizeof(lightprop->GetRadius()));
				}
				else
				{
					int lighttype = -1;
					cscene.write((char*)&lighttype, sizeof(lighttype));
				}

				auto transformprop = object->GetProperty<Transform>();
				if (transformprop)
				{
					cscene.write((char*)&transformprop->Position.x, sizeof(transformprop->Position.x));
					cscene.write((char*)&transformprop->Position.y, sizeof(transformprop->Position.y));
					cscene.write((char*)&transformprop->Position.z, sizeof(transformprop->Position.z));

					cscene.write((char*)&transformprop->Rotation.x, sizeof(transformprop->Rotation.x));
					cscene.write((char*)&transformprop->Rotation.y, sizeof(transformprop->Rotation.y));
					cscene.write((char*)&transformprop->Rotation.z, sizeof(transformprop->Rotation.z));

					cscene.write((char*)&transformprop->Scale.x, sizeof(transformprop->Scale.x));
					cscene.write((char*)&transformprop->Scale.y, sizeof(transformprop->Scale.y));
					cscene.write((char*)&transformprop->Scale.z, sizeof(transformprop->Scale.z));
				}
			}
		}

		cscene.close();
	}

	void Scene::Clean()
	{
		std::string cscenefile = mDirectory + "\\" + mName + "\\" + mName + ".cscene";
		std::ifstream cscene(cscenefile, std::ios::in | std::ios::binary);
		
		uint32_t sceneobjectssize;
		cscene.read((char*)&sceneobjectssize, sizeof(sceneobjectssize));

		for (uint32_t i = sceneobjectssize; i < mSceneObjects.size(); i++)
		{
			Skybox* skybox = mSceneObjects[i]->GetProperty<Skybox>();
			if (skybox)
			{
				ghc::filesystem::remove(skybox->GetFilepath());
			}

			Model* model = mSceneObjects[i]->GetProperty<Model>();
			if (model)
			{
				std::string cmodelfile = mDirectory + "\\" + mName + "\\" + "Assets\\" + model->Name + ".cmodel";
				std::ifstream cmodel(cmodelfile, std::ios::in | std::ios::binary);

				uint32_t materialsize;
				cmodel.read((char*)&materialsize, sizeof(materialsize));

				std::string tex;
				uint32_t temp;

				for (uint32_t i = 0; i < materialsize; i++)
				{
					uint32_t diffusemapnamesize;
					cmodel.read((char*)&diffusemapnamesize, sizeof(diffusemapnamesize));
					tex.resize(diffusemapnamesize);
					cmodel.read((char*)tex.data(), diffusemapnamesize);
					if (!tex.empty()) { ghc::filesystem::remove(tex); }

					cmodel.read((char*)&temp, sizeof(temp));
					cmodel.read((char*)&temp, sizeof(temp));
					cmodel.read((char*)&temp, sizeof(temp));
					cmodel.read((char*)&temp, sizeof(temp));

					uint32_t normalmapnamesize;
					cmodel.read((char*)&normalmapnamesize, sizeof(normalmapnamesize));
					tex.resize(normalmapnamesize);
					cmodel.read((char*)tex.data(), normalmapnamesize);
					if (!tex.empty()) { ghc::filesystem::remove(tex); }

					cmodel.read((char*)&temp, sizeof(temp));
					cmodel.read((char*)&temp, sizeof(temp));
					cmodel.read((char*)&temp, sizeof(temp));
					cmodel.read((char*)&temp, sizeof(temp));
				}

				cmodel.close();
				ghc::filesystem::remove(cmodelfile);
			}

		}

		cscene.close();
	}
}