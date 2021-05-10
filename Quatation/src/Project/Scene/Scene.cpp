#include "Scene.h"

#include <glad/glad.h>
#include "BinaryHelper.h"

namespace choice
{
	Scene::Scene(const std::string& name, const std::string& directory)
		:mName(name), mDirectory(directory)
	{
		ghc::filesystem::create_directory(mDirectory + "\\" + mName);
		ghc::filesystem::create_directory(mDirectory + "\\" + mName + "\\" + "Assets");

		ghc::filesystem::copy("Choice/assets/hdri_skybox/Road_to_MonumentValley_Ref.hdr", 
			mDirectory + "\\" + mName + "\\" + "Assets\\Road_to_MonumentValley_Ref.hdr");

		SceneObject* object = new SceneObject();
		std::string dstHDRI = mDirectory + "\\" + mName + "\\" + "Assets\\Road_to_MonumentValley_Ref.hdr";
		object->AddProperty<Skybox>(new Skybox(dstHDRI));
		mSceneObjects.push_back(object);

		SceneObject* _object = new SceneObject();

		DirectionalLight* directionallight = new DirectionalLight();
		directionallight->Name = "Directional Light";
		directionallight->Type = LightType::DIRECTIONAL;
		_object->AddProperty<Light>(directionallight);

		Transform* transform = new Transform();
		transform->Position = { 0.0f, 0.0f, 0.0f };
		transform->Rotation = { 0.0f, 0.0f, 0.0f };
		transform->Scale = { 1.0f, 1.0f, 1.0f };
		_object->AddProperty<Transform>(transform);
		mSceneObjects.push_back(_object);

		mBoundingBox = CalculateBoundingBox(nullptr, 0, 0);
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

		mBoundingBox = CalculateBoundingBox(nullptr, 0, 0);

		uint32_t sceneobjectssize;
		Binary::Read<uint32_t>(containedscene, sceneobjectssize);
		mSceneObjects.resize(sceneobjectssize);
		for (auto& object : mSceneObjects)
		{
			object = new SceneObject();

			std::string skyboxsrcFile;
			Binary::Read<std::string>(containedscene, skyboxsrcFile);
			if (!skyboxsrcFile.empty())
			{
				object->AddProperty<Skybox>(new Skybox(skyboxsrcFile));
			}

			int drawabletype;
			Binary::Read<int>(containedscene, drawabletype);
			if (drawabletype != -1)
			{
				switch (drawabletype)
				{
				case 0: //Model
					{
						std::string modelsrcFile;
						Binary::Read<std::string>(containedscene, modelsrcFile);
						object->AddProperty<Drawable>(LoadDrawable(modelsrcFile, DrawableType::MODEL, false));
					}
					break;
				case 1: //Cube
					{
						std::string name;
						Binary::Read<std::string>(containedscene, name);
						object->AddProperty<Drawable>(LoadDrawable(name, DrawableType::CUBE, false));
					}
					break;
				case 2: //Sphere
					{
						std::string name;
						Binary::Read<std::string>(containedscene, name);
						object->AddProperty<Drawable>(LoadDrawable(name, DrawableType::SPHERE, false));
					}
					break;
				}

				//Read Material Info
				Drawable* drawable = object->GetProperty<Drawable>();
				if (drawable)
				{
					LoadMaterials(containedscene, drawable->GetMaterials());
				}
			}
			
			int lightype;
			Binary::Read<int>(containedscene, lightype);
			if (lightype != -1)
			{
				std::string lightname;
				Binary::Read<std::string>(containedscene, lightname);

				glm::vec3 color;
				Binary::Read<glm::vec3>(containedscene, color);

				float intensity;
				Binary::Read<float>(containedscene, intensity);

				float radius;
				Binary::Read<float>(containedscene, radius);

				switch (lightype)
				{
				case 0:
					{
						DirectionalLight* directionallight = new DirectionalLight();
						directionallight->Name = lightname;
						directionallight->Type = LightType::DIRECTIONAL;
						directionallight->Color = color;
						directionallight->Intensity = intensity;
						directionallight->Radius = radius;
						object->AddProperty<Light>(directionallight);
					}
					break;
				case 1:
					{
						PointLight* pointlight = new PointLight();
						pointlight->Name = lightname;
						pointlight->Type = LightType::POINT;
						pointlight->Color = color;
						pointlight->Intensity = intensity;
						pointlight->Radius = radius;
						object->AddProperty<Light>(pointlight);
					}
					break;
				}
			}

			if (drawabletype != -1 || lightype != -1)
			{
				Transform* transform = new Transform();

				Binary::Read<glm::vec3>(containedscene, transform->Position);
				Binary::Read<glm::vec3>(containedscene, transform->Rotation);
				Binary::Read<glm::vec3>(containedscene, transform->Scale);

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
		Binary::Write<uint32_t>(cscene, sceneobjectssize);
		for (auto& object : mSceneObjects)
		{
			if (object)
			{
				auto skyboxprop = object->GetProperty<Skybox>();
				if (skyboxprop)
				{
					std::string t = skyboxprop->GetFilepath();
					Binary::Write<std::string>(cscene, t);
				}
				else
				{
					uint32_t skyboxsrcFilesize = 0;
					Binary::Write<uint32_t>(cscene, skyboxsrcFilesize);
				}

				auto drawableprop = object->GetProperty<Drawable>();
				if (drawableprop)
				{
					int drawabletype = static_cast<int>(drawableprop->GetDrawableType());
					Binary::Write<int>(cscene, drawabletype);

					switch (drawabletype)
					{
					case 0: //Model
						//Write Model Filepath
						{
							std::string modelsrcFile = mDirectory + "\\" + mName + "\\" + "Assets\\" + drawableprop->GetName() + ".cmodel";
							Binary::Write<std::string>(cscene, modelsrcFile);
						}
						break;
					case 1: //Cube
					case 2: //Sphere
						//Write Name
						Binary::Write<std::string>(cscene, drawableprop->GetName());
						break;
					}

					//Write Material Info
					uint32_t materialssize = (uint32_t)drawableprop->GetMaterials().size();
					cscene.write((char*)&materialssize, sizeof(materialssize));
					for (auto& material : drawableprop->GetMaterials())
					{
						Binary::Write<std::string>(cscene, material->Name);

						Binary::Write<float>(cscene, material->Roughness);
						Binary::Write<float>(cscene, material->Metallic);
						Binary::Write<float>(cscene, material->Ao);

						Binary::Write<glm::vec4>(cscene, material->Color);

						SaveMaterialsData(cscene, material->DiffuseMap.second.second);
						SaveMaterialsData(cscene, material->NormalMap.second.second);
						SaveMaterialsData(cscene, material->RoughnessMap.second.second);
						SaveMaterialsData(cscene, material->MetallicMap.second.second);
						SaveMaterialsData(cscene, material->AOMap.second.second);
					}
				}
				else
				{
					int drawabletype = -1;
					Binary::Write<int>(cscene, drawabletype);
				}

				auto lightprop = object->GetProperty<Light>();
				if (lightprop)
				{
					int lighttype = static_cast<int>(lightprop->Type);
					Binary::Write<int>(cscene, lighttype);

					Binary::Write<std::string>(cscene, lightprop->Name);

					Binary::Write<glm::vec3>(cscene, lightprop->Color);

					Binary::Write<float>(cscene, lightprop->Intensity);
					Binary::Write<float>(cscene, lightprop->Radius);
				}
				else
				{
					int lighttype = -1;
					Binary::Write<int>(cscene, lighttype);
				}

				auto transformprop = object->GetProperty<Transform>();
				if (transformprop)
				{
					Binary::Write<glm::vec3>(cscene, transformprop->Position);
					Binary::Write<glm::vec3>(cscene, transformprop->Rotation);
					Binary::Write<glm::vec3>(cscene, transformprop->Scale);
				}
			}
		}

		cscene.close();
	}

	void Scene::Clean()
	{
		//TODO : Fix For CUBE And SPHERE
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

			Drawable* drawable = mSceneObjects[i]->GetProperty<Drawable>();
			if (drawable)
			{
				if (drawable->GetDrawableType() == DrawableType::MODEL)
				{
					ghc::filesystem::remove(mDirectory + "\\" + mName + "\\" + "Assets\\" + drawable->GetName() + ".cmodel");
					ghc::filesystem::remove(mDirectory + "\\" + mName + "\\" + "Assets\\" + drawable->GetName() + ".cmaterial");
				}
				for (auto& material : drawable->GetMaterials())
				{
					if (material->DiffuseMap.second.second)
					{
						if (!material->DiffuseMap.second.second->Source.empty())
						{
							ghc::filesystem::remove(material->DiffuseMap.second.second->Source);
						}
					}
					if (material->NormalMap.second.second)
					{
						if (!material->NormalMap.second.second->Source.empty())
						{
							ghc::filesystem::remove(material->NormalMap.second.second->Source);
						}
					}
				}
			}
		}

		cscene.close();
	}
}