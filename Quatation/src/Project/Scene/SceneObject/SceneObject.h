#pragma once
#include "cpch.h"
#include "Property/Model.h"

namespace choice
{
	class SceneObject
	{
	public:
		~SceneObject()
		{
			if (mModel.has_value()) { delete mModel.value(); }
		}

		template<typename T>
		void AddProperty(T* property) { static_assert(false); }

		template<>
		void AddProperty<Model>(Model* model)
		{
			if (!mModel.has_value()) { mModel.emplace(model); return; }
			std::cout << "Property Already Exists" << std::endl;
		}

		template<typename T>
		T* GetProperty() { static_assert(false); }

		template<>
		Model* GetProperty<Model>() { if (mModel.has_value()) { return mModel.value(); } return nullptr; }
	private:
		std::optional<Model*> mModel;
	};
}