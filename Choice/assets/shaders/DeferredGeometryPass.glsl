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

layout(location = 0)out vec3 gPosition;
layout(location = 1)out vec3 gNormal;
layout(location = 2)out vec4 gAlbedoS;
layout(location = 3)out vec3 gRoughMetalAo; 
layout(location = 4)out vec3 gPixelInfo;

struct Material
{
	sampler2D Diffuse;
	sampler2D Normal;
	sampler2D Roughness;
	sampler2D Metallic;
	sampler2D AmbientOcclusion;
	float RoughnessFactor;
	float MetallicFactor;
	float AO;
	vec4 Color;
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
uniform int gHasRoughnessMap;
uniform int gHasMetallicMap;
uniform int gHasAmbientOcclusionMap;
uniform int gNodeIndex;
uniform int gDrawIndex;

void main()
{
	gPosition = fs_in.vFragPos; //Fragment Position

	if(gHasNormalMap == 1)
	{
		vec3 tangentNormal = texture(gMaterial.Normal, fs_in.vTexCoords).xyz * 2.0 - 1.0;

		vec3 Q1  = dFdx(fs_in.vFragPos);
		vec3 Q2  = dFdy(fs_in.vFragPos);
		vec2 st1 = dFdx(fs_in.vTexCoords);
		vec2 st2 = dFdy(fs_in.vTexCoords);

		vec3 N   = normalize(fs_in.vNormal);
		vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
		vec3 B  = -normalize(cross(N, T));
		mat3 TBN = mat3(T, B, N);

		gNormal = normalize(TBN * tangentNormal); //Normals From NormalMap
	}
	else
	{
		gNormal = fs_in.vNormal; //Normals From Vertex Attributes
	}

	if(gHasDiffuseMap == 1)
	{
		gAlbedoS.rgb = texture(gMaterial.Diffuse, fs_in.vTexCoords).rgb;
		gAlbedoS.a = 1.0;
	}
	else
	{
		gAlbedoS = gMaterial.Color;
	}
	
	float Roughness;
	if(gHasRoughnessMap == 1)
	{
		Roughness = texture(gMaterial.Roughness, fs_in.vTexCoords).r * gMaterial.RoughnessFactor;
	}
	else
	{
		Roughness = gMaterial.RoughnessFactor;
	}

	float Metallic;
	if(gHasMetallicMap == 1)
	{
		Metallic = texture(gMaterial.Metallic, fs_in.vTexCoords).r * gMaterial.MetallicFactor;
	}
	else
	{
		Metallic = gMaterial.MetallicFactor;
	}

	float AO;
	if(gHasAmbientOcclusionMap == 1)
	{
		AO = texture(gMaterial.AmbientOcclusion, fs_in.vTexCoords).r * gMaterial.AO;
	}
	else
	{
		AO = gMaterial.AO;
	}

	gRoughMetalAo = vec3(Roughness, Metallic, AO);

	gPixelInfo = vec3(float(gNodeIndex), float(gDrawIndex), float(gl_PrimitiveID + 1));
}