#include "Light.h"

#include "ReflectionData.h"

namespace choice
{

	DirectionalLight::DirectionalLight()
	{
		auto& layout = global::GlobalReflectionData.UniformBuffers["Lights"]->GetBufferLayout();
		Data.resize(layout.find("Lights.ldLights[0]")->second->size);
		Type = LIGHT_TYPE::DIRECTIONAL;
		node_data_type = NODE_DATA_TYPE::LIGHT;

		//Set Default Light Color As White
		glm::vec3* color = global::GlobalReflectionData.UniformBuffers["Lights"]->MemberData<glm::vec3>("Lights.ldLights.Color", Data);
		*color = { 1.0f, 1.0f, 1.0f };
	}

	//View Projection Directional Light
	std::vector<glm::mat4> DirectionalLight::ViewProjection(BoundingBox* sceneaabb)
	{
		Transform* transform = NodeTransform;

		//Light View Matrix
		glm::vec3 direction = glm::normalize(transform->GetTransform()[2]);
		glm::vec3 cross = glm::normalize(glm::cross({ 0.0f, 1.0f, 0.0f }, direction));
		glm::vec3 up = glm::normalize(glm::cross(direction, cross));
		glm::mat4 view = glm::lookAt(glm::normalize(transform->Position), direction, up);

		//Calculate Scene AABB In Light Space
		BoundingBox lightspaceAABB = CalculateBoundingBox(nullptr, 0, 0);
		std::vector<glm::vec3> expanded = ExpandAABB(*sceneaabb);

		for (auto& point : expanded)
		{
			const glm::vec4 pointinview = view * glm::vec4(point, 1.0f);

			lightspaceAABB.Min.x = pointinview.x < lightspaceAABB.Min.x ? pointinview.x : lightspaceAABB.Min.x;
			lightspaceAABB.Min.y = pointinview.y < lightspaceAABB.Min.y ? pointinview.y : lightspaceAABB.Min.y;
			lightspaceAABB.Min.z = pointinview.z < lightspaceAABB.Min.z ? pointinview.z : lightspaceAABB.Min.z;

			lightspaceAABB.Max.x = pointinview.x > lightspaceAABB.Max.x ? pointinview.x : lightspaceAABB.Max.x;
			lightspaceAABB.Max.y = pointinview.y > lightspaceAABB.Max.y ? pointinview.y : lightspaceAABB.Max.y;
			lightspaceAABB.Max.z = pointinview.z > lightspaceAABB.Max.z ? pointinview.z : lightspaceAABB.Max.z;
		}

		//Light Projection Matrix 
		glm::mat4 projection = glm::ortho(lightspaceAABB.Min.x, lightspaceAABB.Max.x, 
										  lightspaceAABB.Min.y, lightspaceAABB.Max.y, 
										  lightspaceAABB.Min.z, lightspaceAABB.Max.z);

		std::vector<glm::mat4> viewprojection(1);
		viewprojection[0] = projection * view;
		return viewprojection;
	}

	PointLight::PointLight()
	{
		auto& layout = global::GlobalReflectionData.UniformBuffers["Lights"]->GetBufferLayout();
		Data.resize(layout.find("Lights.lpLights[0]")->second->size);
		Type = LIGHT_TYPE::POINT;
		node_data_type = NODE_DATA_TYPE::LIGHT;

		//Set Default Light Color As White
		glm::vec3* color = global::GlobalReflectionData.UniformBuffers["Lights"]->MemberData<glm::vec3>("Lights.lpLights.Color", Data);
		*color = { 1.0f, 1.0f, 1.0f };
	}

	//View Projection Point Light
	std::vector<glm::mat4> PointLight::ViewProjection(BoundingBox* sceneaabb)
	{
		std::vector<glm::mat4> viewprojection(1);
		viewprojection[0] = glm::mat4(1.0f);
		return viewprojection;
	}

	Light* CreateDiretionalLight(const std::string& name /*= "Directional Light"*/)
	{
		Light* directionallight = new DirectionalLight();
		directionallight->Name = name;

		return directionallight;
	}

	Light* CreatePointLight(const std::string& name /*= "Point Light"*/)
	{
		Light* pointlight = new PointLight();
		pointlight->Name = name;

		return pointlight;
	}

	Light::~Light()
	{
		Data.clear();
	}

}