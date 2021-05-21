#source vertex
#version 450 core

layout(location = 0)in vec3 aPosition;
layout(location = 1)in vec3 aNormal;
layout(location = 2)in vec2 aTexCoords;
layout(location = 3)in vec3 aTangent;

from UniformBuffers.glsl include uniform Camera,uniform Transform;

from Structures.glsl include struct GPVertexOutput;

layout(location = 0)out GPVertexOutput vOutput;

void main()
{
	vOutput.FragPos = (uTransform * vec4(aPosition, 1.0)).xyz;
	vOutput.Normal = aNormal;
	vOutput.TexCoords = aTexCoords;
	gl_Position = uViewProjection * uTransform * vec4(aPosition, 1.0);
}

#source fragment
#version 450 core

layout(location = 0)out vec3 gPosition;
layout(location = 1)out vec3 gNormal;
layout(location = 2)out vec4 gAlbedoS;
layout(location = 3)out vec3 gRoughMetalAo; 
layout(location = 4)out vec3 gPixelInfo;

from Structures.glsl include struct GPVertexOutput;

layout(location = 0)in GPVertexOutput gInput;

from UniformBuffers.glsl include uniform Material;

layout(binding = 0)uniform sampler2D gAlbedoMap;
layout(binding = 1)uniform sampler2D gNormalMap;
layout(binding = 2)uniform sampler2D gRoughnessMap;
layout(binding = 3)uniform sampler2D gMetallicMap;
layout(binding = 4)uniform sampler2D gAoMap;

void main()
{
	gPosition = gInput.FragPos; //Fragment Position

	if(HasNormalMap == 1)
	{
		vec3 tangentNormal = texture(gNormalMap, gInput.TexCoords).xyz * 2.0 - 1.0;

		vec3 Q1  = dFdx(gInput.FragPos);
		vec3 Q2  = dFdy(gInput.FragPos);
		vec2 st1 = dFdx(gInput.TexCoords);
		vec2 st2 = dFdy(gInput.TexCoords);

		vec3 N   = normalize(gInput.Normal);
		vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
		vec3 B  = -normalize(cross(N, T));
		mat3 TBN = mat3(T, B, N);

		gNormal = normalize(TBN * tangentNormal); //Normals From NormalMap
	}
	else
	{
		gNormal = gInput.Normal; //Normals From Vertex Attributes
	}

	if(HasAlbedoMap == 1)
	{
		gAlbedoS.rgb = texture(gAlbedoMap, gInput.TexCoords).rgb;
		gAlbedoS.a = 1.0;
	}
	else
	{
		gAlbedoS = Color;
	}
	
	float roughness;
	if(HasRoughnessMap == 1)
	{
		roughness = texture(gRoughnessMap, gInput.TexCoords).r * Roughness;
	}
	else
	{
		roughness = Roughness;
	}

	float metallic;
	if(HasMetallicMap == 1)
	{
		metallic = texture(gMetallicMap, gInput.TexCoords).r * Metallic;
	}
	else
	{
		metallic = Metallic;
	}

	float ao;
	if(HasAoMap == 1)
	{
		ao = texture(gAoMap, gInput.TexCoords).r;
	}
	else
	{
		ao = 1.0;
	}

	gRoughMetalAo = vec3(roughness, metallic, ao);

	gPixelInfo = vec3(0.0, 0.0, 0.0);
}