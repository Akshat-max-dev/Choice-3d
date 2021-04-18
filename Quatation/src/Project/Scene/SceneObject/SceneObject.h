#pragma once
#include "cpch.h"
#include "Property/Model.h"
#include "Property/Transform.h"
#include "Property/Skybox.h"
#include "Property/Light.h"

namespace choice
{
	class SceneObject
	{
	public:
		~SceneObject()
		{
			if (mModel.has_value()) { delete mModel.value(); }
			if (mTransform.has_value()) { delete mTransform.value(); }
			if (mSkybox.has_value()) { delete mSkybox.value(); }
		}

		std::string& Name() { return mName; }

		template<typename T>
		void AddProperty(T* property) { static_assert(false); }

		template<>
		void AddProperty<Model>(Model* model)
		{
			if (!mModel.has_value()) { mModel.emplace(model); mName = model->Name; return; }
			std::cout << "Property Already Exists" << std::endl;
		}

		template<>
		void AddProperty<Transform>(Transform* transform)
		{
			if (!mTransform.has_value()) { mTransform.emplace(transform); return; }
			std::cout << "Property Already Exists" << std::endl;
		}

		template<>
		void AddProperty<Skybox>(Skybox* skybox)
		{
			if (!mSkybox.has_value()) { mSkybox.emplace(skybox); 
			mName = ghc::filesystem::path(skybox->GetFilepath()).stem().string(); return; }
			std::cout << "Property Already Exists" << std::endl;
		}

		template<>
		void AddProperty<Light>(Light* light)
		{
			if (!mLight.has_value()) { mLight.emplace(light); mName = light->GetName(); return; }
			std::cout << "Property Already Exists" << std::endl;
		}

		template<typename T>
		void DrawProperty();

		template<>
		void DrawProperty<Model>();

		template<>
		void DrawProperty<Transform>();

		template<>
		void DrawProperty<Skybox>();

		template<>
		void DrawProperty<Light>();

		template<typename T>
		T* GetProperty() { static_assert(false); }

		template<>
		Model* GetProperty<Model>() { if (mModel.has_value()) { return mModel.value(); } return nullptr; }

		template<>
		Transform* GetProperty<Transform>() { if (mTransform.has_value()) { return mTransform.value(); } return nullptr; }
	
		template<>
		Skybox* GetProperty<Skybox>() { if (mSkybox.has_value()) { return mSkybox.value(); } return nullptr; }
	
		template<>
		Light* GetProperty<Light>() { if (mLight.has_value()) { return mLight.value(); } return nullptr; }
	private:
		std::optional<Model*> mModel;
		std::optional<Transform*> mTransform;
		std::optional<Skybox*> mSkybox;
		std::optional<Light*> mLight;
		std::string mName;
	};
}