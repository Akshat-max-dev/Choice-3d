#pragma once
#include "cpch.h"

#include <glm/glm.hpp>

#include "Transform.h"

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

		virtual glm::mat4 View(Transform* transform) { return glm::mat4(1.0f); }

		virtual std::vector<glm::mat4> ViewProjection(Transform* transform) 
		{
			std::vector<glm::mat4> viewprojection; viewprojection.push_back(glm::mat4(1.0f)); return viewprojection;
		}
	protected:
		glm::vec3 mColor = { 1.0f, 1.0f, 1.0f };
		float mIntensity = 1.0f;
		float mRadius = 1.0f;

		LightType mLightType = LightType::NONE;
		std::string mName;

		glm::mat4 mProjection;
		glm::mat4 mView;
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
		
		glm::mat4 View(Transform* transform)override
		{
			glm::vec3 dir = transform->GetTransform()[2];
			return glm::lookAt(transform->Position, dir, {0, 1, 0});
		}

		std::vector<glm::mat4> ViewProjection(Transform* transform)override
		{
			std::vector<glm::mat4> viewprojection;
			mProjection = glm::ortho(-100.0f, 100.0f, 100.0f, -100.0f, 10.0f, 100.0f);
			glm::vec3 dir = transform->GetTransform()[2];
			mView = glm::lookAt(transform->Position, dir, glm::vec3(0, 1, 0));
			viewprojection.push_back(mProjection * mView);
			return viewprojection;
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