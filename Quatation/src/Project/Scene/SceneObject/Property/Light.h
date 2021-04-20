#pragma once
#include "cpch.h"

#include <glm/glm.hpp>

namespace choice
{
	enum class LightType
	{
		NONE = -1, DIRECTIONAL = 0, POINT = 1
	};

	class Light
	{
	public:
		glm::vec3& GetDiffuse() { return mColor; }
		float& GetIntensity() { return mIntensity; }

		const LightType& GetLightType() { return mLightType; }
		std::string& GetName() { return mName; }

		float& GetRadius() { return mRadius; }
	protected:
		glm::vec3 mColor = { 1.0f, 1.0f, 1.0f };
		float mIntensity = 1.0f;
		float mRadius = 1.0f;

		LightType mLightType = LightType::NONE;
		std::string mName;
	};

	class DirectionalLight :public Light 
	{
	public:
		DirectionalLight() { mName = "Directional Light"; mLightType = LightType::DIRECTIONAL; }
		DirectionalLight(const std::string& name, const glm::vec3 color, float intensity)
		{
			mName = name; 
			mColor = color; 
			mIntensity = intensity;
			mLightType = LightType::DIRECTIONAL;
		}
	};

	class PointLight :public Light
	{
	public:
		PointLight() { mName = "Point Light"; mLightType = LightType::POINT; }
		PointLight(const std::string& name, const glm::vec3 color, float intensity, float radius) 
		{
			mName = name; 
			mColor = color;
			mIntensity = intensity;
			mRadius = radius;
			mLightType = LightType::POINT; 
		}
	};
}