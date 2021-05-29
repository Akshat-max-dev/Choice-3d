//All UniformBuffers Here

layout(std140, binding = 0)uniform Camera
{
    mat4 uViewProjection;
};

layout(std140, binding = 1)uniform Transform
{
    mat4 uTransform;
};

layout(std140, binding = 2)uniform Material
{
	vec3 Color;
	float _padding1;
	float Roughness;
	float Metallic;
	int HasAlbedoMap;
	int HasNormalMap;
	int HasRoughnessMap;
	int HasMetallicMap;
	int HasAoMap;
};

layout(std140, binding = 2)uniform Color
{
	vec4 cColor;
};

layout(std140, binding = 0)uniform Lights
{
	DirectionalLight ldLights[8];
	PointLight		 lpLights[32];
	int ldLightsActive;
	int lpLightsActive;
	vec3 lViewpos;
};

layout(std140, binding = 1)uniform Capture
{
    mat4 uProjection;
    mat4 uView;
};

layout(std140, binding = 2)uniform Roughness
{
    float pfRoughness;
};