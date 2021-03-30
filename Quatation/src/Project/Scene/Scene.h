#pragma once
#include "cpch.h"

#include "Model.h"
#include "OpenGL/Pipeline/DeferredPipeline.h"
#include "Camera/EditorCamera.h"

namespace choice
{
	enum class SceneObjectType
	{
		MODEL = 0
	};

	class Scene
	{
	public:
		Scene(const std::string& name, const std::string& directory);
		Scene(const std::string& cscene);
		~Scene();

		template<typename T>
		void AddObject(T* object) { static_assert(false); }

		template<>
		void AddObject<Model>(Model* model)
		{
			const char* id = GenerateIds<Model>();
			mModels.insert({ id, model });
			std::ofstream cscenedata(mDirectory + "\\" + mName + "\\" + mName + ".cscenedata", std::ios::app | std::ios::binary);
			if (cscenedata.fail())
			{
				std::cout << "Error Writing To SceneData File" << std::endl;
				cscenedata.close();
			}
			uint32_t idsize = 2;
			cscenedata.write((char*)&idsize, sizeof(idsize));
			cscenedata.write((char*)id, idsize);

			uint32_t modelnamesize = (uint32_t)model->Name.size();
			cscenedata.write((char*)&modelnamesize, sizeof(modelnamesize));
			cscenedata.write((char*)model->Name.data(), modelnamesize);

			cscenedata.close(); 
		}

		void Save();

		std::string& Name() { return mName; }
		const std::string& Directory()const { return mDirectory; }
	private:
		template<typename T>
		const char* GenerateIds() { static_assert(false); }

		template<>
		const char* GenerateIds<Model>()
		{
			std::string temp = "M" + std::to_string(mModels.size());
			return temp.c_str();
		}
	private:
		std::string mName;
		std::string mDirectory;

		std::map<const char*, Model*> mModels;
	};
}