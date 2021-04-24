#source vertex
#version 450 core

layout(location = 0)in vec3 aPosition;
layout(location = 1)in vec3 aNormal;
layout(location = 2)in vec2 aTexCoords;
layout(location = 3)in vec3 aTangent;

uniform mat4 uViewProjection;
uniform mat4 uTransform;

out VS_OUT
{
	vec3 vFragPos;
	vec3 vNormal;
	vec2 vTexCoords;
}vs_out;

void main()
{
	vs_out.vFragPos = (uTransform * vec4(aPosition, 1.0)).xyz;
	vs_out.vNormal = aNormal;
	vs_out.vTexCoords = aTexCoords;
	gl_Position = uViewProjection * uTransform * vec4(aPosition, 1.0);
}

#source fragment
#version 450 core

layout(location = 0)out vec4 gPosition;
layout(location = 1)out vec4 gNormal;
layout(location = 2)out vec4 gAlbedoS;
layout(location = 3)out vec3 gPixelInfo;

struct Material
{
	sampler2D Diffuse;
	sampler2D Normal;
	float Roughness;
	float Metallic;
};

uniform Material gMaterial;

in VS_OUT
{
	vec3 vFragPos;
	vec3 vNormal;
	vec2 vTexCoords;
}fs_in;

uniform int gHasNormalMap;
uniform int gHasDiffuseMap;
uniform int gObjectIndex;
uniform int gDrawIndex;

void main()
{
	gPosition.rgb = fs_in.vFragPos; //Fragment Position
	gPosition.a = gMaterial.Roughness; //Roughness

	if(gHasNormalMap == 1)
	{
		gNormal.rgb = texture(gMaterial.Normal, fs_in.vTexCoords).rgb; //Normals From NormalMap
	}
	else
	{
		gNormal.rgb = fs_in.vNormal; //Normals From Vertex Attributes
	}

	gNormal.a = gMaterial.Metallic; //Metallic

	if(gHasDiffuseMap == 1)
	{
		gAlbedoS.rgb = texture(gMaterial.Diffuse, fs_in.vTexCoords).rgb;
	}
	else
	{
		gAlbedoS.rgb = vec3(0.5f, 0.0f, 0.0f);
	}
	gAlbedoS.a = 1.0;
	//gAlbedoS.a = texture(gMaterial.Specular, fs_in.vTexCoords).r;

	gPixelInfo = vec3(float(gObjectIndex), float(gDrawIndex), float(gl_PrimitiveID + 1));
}