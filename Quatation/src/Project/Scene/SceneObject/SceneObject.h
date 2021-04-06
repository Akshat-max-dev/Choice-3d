#pragma once
#include "cpch.h"
#include "Property/Model.h"
#include "Property/Transform.h"
#include "Property/Skybox.h"

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

		template<typename T>
		void AddProperty(T* property) { static_assert(false); }

		template<>
		void AddProperty<Model>(Model* model)
		{
			if (!mModel.has_value()) { mModel.emplace(model); return; }
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
			if (!mSkybox.has_value()) { mSkybox.emplace(skybox); return; }
			std::cout << "Property Already Exists" << std::endl;
		}

		template<typename T>
		T* GetProperty() { static_assert(false); }

		template<>
		Model* GetProperty<Model>() { if (mModel.has_value()) { return mModel.value(); } return nullptr; }

		template<>
		Transform* GetProperty<Transform>() { if (mTransform.has_value()) { return mTransform.value(); } return nullptr; }
	
		template<>
		Skybox* GetProperty<Skybox>() { if (mSkybox.has_value()) { return mSkybox.value(); } return nullptr; }
	private:
		std::optional<Model*> mModel;
		std::optional<Transform*> mTransform;
		std::optional<Skybox*> mSkybox;
	};
}