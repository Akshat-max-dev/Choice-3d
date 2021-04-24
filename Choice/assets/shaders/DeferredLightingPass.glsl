#source vertex
#version 450 core

out vec2 vTexCoords;

void main()
{
	float x = -1.0 + float((gl_VertexID & 1) << 2);
	float y = -1.0 + float((gl_VertexID & 2) << 1);
	float uvx = (x+1.0)*0.5;
	float uvy = (y+1.0)*0.5;
	vTexCoords.x = uvx;
	vTexCoords.y = uvy;
	gl_Position = vec4(x, y, 0.0, 1.0);
}

#source fragment
#version 450 core

out vec4 lResult;

in vec2 vTexCoords;

struct GBuffer
{
	sampler2D Position;
	sampler2D Normal;
	sampler2D AlbedoS;
};

uniform GBuffer lGBuffer;

struct DirectionalLight
{
	vec3 Direction;
	vec3 Diffuse;
	vec3 Specular;
};

struct PointLight
{
	vec3 Position;
	vec3 Diffuse;
	vec3 Specular;
	
	float Radius;
};

uniform DirectionalLight ldLights[8];
uniform PointLight       lpLights[32];
uniform vec3 lViewpos;
uniform int ldLightsActive;
uniform int lpLightsActive;

void main()
{
	vec3 FragPos = texture(lGBuffer.Position, vTexCoords).rgb;
	vec3 Normal = texture(lGBuffer.Normal, vTexCoords).rgb;
	vec3 Diffuse = texture(lGBuffer.AlbedoS, vTexCoords).rgb;
	float Specular = texture(lGBuffer.AlbedoS, vTexCoords).a;
	float Roughness = texture(lGBuffer.Position, vTexCoords).a;
	float Metallic = texture(lGBuffer.Normal, vTexCoords).a;

	vec3 Lighting = Diffuse;
	vec3 ViewDir = normalize(lViewpos - FragPos);
	Normal = normalize(Normal);

	for(int i = 0; i < ldLightsActive; i++)
	{
		vec3 lightDir = normalize(-ldLights[i].Direction);
		vec3 diffuse = ldLights[i].Diffuse * max(dot(Normal, lightDir), 0.0) * Diffuse;
		
		vec3 halfwayDir = normalize(lightDir + ViewDir);  
		float spec = pow(max(dot(Normal, halfwayDir), 0.0), 32.0);
		vec3 specular = ldLights[i].Specular * spec * Specular;

		Lighting += diffuse + specular;
	}

	for(int i = 0; i < lpLightsActive; i++)
	{
		vec3 lightDir = normalize(lpLights[i].Position - FragPos);
		vec3 diffuse = lpLights[i].Diffuse * max(dot(Normal, lightDir), 0.0) * Diffuse;

		vec3 reflectDir = reflect(-lightDir, Normal);
		float spec = pow(max(dot(ViewDir, reflectDir), 0.0), 32.0);
		vec3 specular = lpLights[i].Specular * spec * Specular;

		float distance = length(lpLights[i].Position - FragPos);
		float attenuation = 0.0;
		if(lpLights[i].Radius != 0)
		{
			attenuation = 2.0/((distance * distance) + (lpLights[i].Radius * lpLights[i].Radius) + 
								distance * (sqrt((distance * distance) + (lpLights[i].Radius * lpLights[i].Radius))));
		}

		diffuse *= attenuation;
		specular *= attenuation;

		Lighting += diffuse + specular;
	}
	
	lResult = vec4(Lighting, 1.0);
}