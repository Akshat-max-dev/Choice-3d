//All UniformBuffers Here

layout(std140, binding = 0)uniform Camera
{
    mat4 Projection;
	mat4 ProjectionInv;
	mat4 View;
	mat4 ViewInv;
	vec3 Position;
}camera;

layout(std140, binding = 1)uniform Transform
{
    mat4 uTransform;
};

layout(std140, binding = 2)uniform Material
{
	vec3 Color;
	int IsPBR;
	float Roughness;
	float Metallic;
	int HasAlbedoMap;
	int HasNormalMap;
	int HasRoughnessMap;
	int HasMetallicMap;
	int HasAoMap;
	int HasDisplacementMap;
};

layout(std140, binding = 3)uniform DisplacementMap
{
	int HasDisplacementMap;
};

layout(std140, binding = 2)uniform Color
{
	vec4 cColor;
};

layout(std140, binding = 1)uniform Lights
{
	DirectionalLight ldLights[8];
	PointLight		 lpLights[32];
	int ldLightsActive;
	int lpLightsActive;
};

layout(std140, binding = 1)uniform Roughness
{
    float pfRoughness;
};