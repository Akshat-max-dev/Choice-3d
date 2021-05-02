#include "Scene.h"

#include <glad/glad.h>

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

			int drawabletype;
			containedscene.read((char*)&drawabletype, sizeof(drawabletype));
			if (drawabletype != -1)
			{
				switch (drawabletype)
				{
				case 0: //Model
					{
						uint32_t modelsrcFileSize;
						containedscene.read((char*)&modelsrcFileSize, sizeof(modelsrcFileSize));
						std::string modelsrcFile;
						modelsrcFile.resize(modelsrcFileSize);
						containedscene.read((char*)modelsrcFile.data(), modelsrcFileSize);
						object->AddProperty<Drawable>(LoadDrawable(modelsrcFile, DrawableType::MODEL, false));
					}
					break;
				case 1: //Cube
					{
						uint32_t namesize;
						containedscene.read((char*)&namesize, sizeof(namesize));
						std::string name;
						name.resize(namesize);
						containedscene.read((char*)name.data(), namesize);
						object->AddProperty<Drawable>(LoadDrawable(name, DrawableType::CUBE, false));
					}
					break;
				case 2: //Sphere
					{
						uint32_t namesize;
						containedscene.read((char*)&namesize, sizeof(namesize));
						std::string name;
						name.resize(namesize);
						containedscene.read((char*)name.data(), namesize);
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

				auto drawableprop = object->GetProperty<Drawable>();
				if (drawableprop)
				{
					int drawabletype = static_cast<int>(drawableprop->GetDrawableType());
					cscene.write((char*)&drawabletype, sizeof(drawabletype));

					switch (drawabletype)
					{
					case 0: //Model
						//Write Model Filepath
						{
							std::string modelsrcFile = mDirectory + "\\" + mName + "\\" + "Assets\\" + drawableprop->GetName() + ".cmodel";
							uint32_t modelsrcFileSize = (uint32_t)modelsrcFile.size();
							cscene.write((char*)&modelsrcFileSize, sizeof(modelsrcFileSize));
							cscene.write((char*)modelsrcFile.data(), modelsrcFileSize);
						}
						break;
					case 1: //Cube
					case 2: //Sphere
						//Write Name
						uint32_t namesize = (uint32_t)drawableprop->GetName().size();
						cscene.write((char*)&namesize, sizeof(namesize));
						cscene.write((char*)drawableprop->GetName().data(), namesize);
						break;
					}

					//Write Material Info
					uint32_t materialssize = (uint32_t)drawableprop->GetMaterials().size();
					cscene.write((char*)&materialssize, sizeof(materialssize));
					for (auto& material : drawableprop->GetMaterials())
					{
						uint32_t materialnamesize = (uint32_t)material->Name.size();
						cscene.write((char*)&materialnamesize, sizeof(materialnamesize));
						cscene.write((char*)material->Name.data(), materialnamesize);

						cscene.write((char*)&material->Roughness, sizeof(material->Roughness));
						cscene.write((char*)&material->Metallic, sizeof(material->Metallic));
						cscene.write((char*)&material->Ao, sizeof(material->Ao));

						cscene.write((char*)&material->Color.r, sizeof(material->Color.r));
						cscene.write((char*)&material->Color.g, sizeof(material->Color.g));
						cscene.write((char*)&material->Color.b, sizeof(material->Color.b));
						cscene.write((char*)&material->Color.a, sizeof(material->Color.a));

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
					cscene.write((char*)&drawabletype, sizeof(drawabletype));
				}

				auto lightprop = object->GetProperty<Light>();
				if (lightprop)
				{
					int lighttype = static_cast<int>(lightprop->Type);
					cscene.write((char*)&lighttype, sizeof(lighttype));

					uint32_t lightnamesize = (uint32_t)lightprop->Name.size();
					cscene.write((char*)&lightnamesize, sizeof(lightnamesize));
					cscene.write((char*)lightprop->Name.data(), lightnamesize);

					cscene.write((char*)&lightprop->Color.x, sizeof(lightprop->Color.x));
					cscene.write((char*)&lightprop->Color.y, sizeof(lightprop->Color.y));
					cscene.write((char*)&lightprop->Color.z, sizeof(lightprop->Color.z));

					cscene.write((char*)&lightprop->Intensity, sizeof(lightprop->Intensity));
					cscene.write((char*)&lightprop->Radius, sizeof(lightprop->Radius));
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