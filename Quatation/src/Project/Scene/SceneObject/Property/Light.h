#pragma once
#include "cpch.h"

namespace choice
{
	enum class LightType
	{
		NONE = -1, DIRECTIONAL = 0, POINT = 1
	};

	class Light
	{
	public:
		const LightType& GetLightType() { return mLightType; }
		std::string& GetName() { return mName; }
	protected:
		LightType mLightType = LightType::NONE;
		std::string mName;
	};

	class DirectionalLight :public Light 
	{
	public:
		DirectionalLight() { mName = "Directional Light"; mLightType = LightType::DIRECTIONAL; }
		DirectionalLight(const std::string& name) { mName = name; }
	};

	class PointLight :public Light
	{
	public:
		PointLight() { mName = "Point Light"; mLightType = LightType::POINT; }
		PointLight(const std::string& name) { mName = name; }
	};
}