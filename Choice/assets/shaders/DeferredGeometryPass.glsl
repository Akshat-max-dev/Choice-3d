#source vertex
#version 450 core

layout(location = 0)in vec3 aPosition;
layout(location = 1)in vec3 aNormal;
layout(location = 2)in vec2 aTexCoords;
layout(location = 3)in vec3 aTangent;

from UniformBuffers.glsl include uniform Camera,uniform Transform,uniform DisplacementMap;

layout(binding = 5)uniform sampler2D gDisplacementMap;

from Structures.glsl include struct GPVertexOutput;

layout(location = 0)out GPVertexOutput vOutput;

void main()
{
	vec3 displacedPosition = aPosition;
	if(HasDisplacementMap == 1)
	{
		float displacementFactor = textureLod(gDisplacementMap, aTexCoords, 0).r;
		displacedPosition = (aNormal * displacementFactor) + displacedPosition;
	}

	vOutput.FragPos = (uTransform * vec4(displacedPosition, 1.0)).xyz;
	vOutput.Normal = (uTransform * vec4(aNormal, 0.0)).xyz;
	vOutput.TexCoords = aTexCoords;
	vOutput.ViewPos = camera.Position;
	gl_Position = camera.Projection * camera.View * uTransform * vec4(displacedPosition, 1.0);
}

#source fragment
#version 450 core

layout(location = 0)out vec4 gNormal;
layout(location = 1)out vec4 gAlbedoS;
layout(location = 2)out vec3 gRoughMetalAo; 

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
	if(HasNormalMap == 1)
	{
		vec3 normal = vec3(0.0);
		normal.xy = texture(gNormalMap, gInput.TexCoords).xy;
		normal.z = sqrt(1.0 - (normal.x * normal.x) - (normal.y * normal.y));

		vec3 tangentNormal = normal * 2.0 - 1.0;

		vec3 Q1  = dFdx(gInput.FragPos);
		vec3 Q2  = dFdy(gInput.FragPos);
		vec2 st1 = dFdx(gInput.TexCoords);
		vec2 st2 = dFdy(gInput.TexCoords);

		vec3 N   = normalize(gInput.Normal);
		vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
		vec3 B  = -normalize(cross(N, T));
		mat3 TBN = mat3(T, B, N);

		gNormal.rgb = normalize(TBN * tangentNormal); //Normals From NormalMap
		gNormal.a = float(IsPBR);
	}
	else
	{
		gNormal.rgb = gInput.Normal; //Normals From Vertex Attributes
		gNormal.a = float(IsPBR);
	}

	if(HasAlbedoMap == 1)
	{
		gAlbedoS.rgb = texture(gAlbedoMap, gInput.TexCoords).rgb;
		gAlbedoS.a = 1.0;
	}
	else
	{
		gAlbedoS.rgb = Color;
		gAlbedoS.a = 1.0;
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
}