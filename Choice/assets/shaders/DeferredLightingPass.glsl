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
	sampler2D RoughMetalAo;
};

uniform GBuffer lGBuffer;

struct IBL
{
    samplerCube IrradianceMap;
    samplerCube PreFilterMap;
    sampler2D   BRDFLookup;
};

uniform IBL lIBL;

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

uniform mat4 uLightViewProjection;
uniform sampler2DShadow lShadowMap;

float CalculateShadows(vec4 fragPosLightSpace)
{
	vec3 ProjCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    vec2 UVCoords;
    UVCoords.x = 0.5 * ProjCoords.x + 0.5;
    UVCoords.y = 0.5 * ProjCoords.y + 0.5;
    float z    = 0.5 * ProjCoords.z + 0.5;

	vec2 texelSize = textureSize(lShadowMap, 0);

    float xOffset = 1.0/texelSize.x;
    float yOffset = 1.0/texelSize.y;

    float Factor = 0.0;

	if(z > 1.0)
	{
		return 1.0;
	}

    for (int y = -1 ; y <= 1 ; y++) {
        for (int x = -1 ; x <= 1 ; x++) {
            vec2 Offsets = vec2(x * xOffset, y * yOffset);
            vec3 UVC = vec3(UVCoords + Offsets, (z - 0.05) + 0.001);
            Factor += texture(lShadowMap, UVC);
        }
    }

    return (0.5 + (Factor / 9.0));
}

const float PI = 3.14159265359;

// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.001); // prevent divide by zero for roughness=0.0 and NdotH=1.0
}

// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// ----------------------------------------------------------------------------
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

// ----------------------------------------------------------------------------
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}   

// ----------------------------------------------------------------------------
vec3 CalculateLo(vec3 L, vec3 N, vec3 V, vec3 Ra, vec3 F0, float R, float M, vec3 A)
{
	vec3 H = normalize(V + L); //Halfway Vector

	//Cook-Torrance BRDF
	float D = DistributionGGX(N, H, R);
	float G = GeometrySmith(N, V, L, R);
	vec3  F = FresnelSchlick(max(dot(H, V), 0.0), F0);

	vec3 Nominator    = D * G * F;
	float Denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
	vec3 Specular	  = Nominator / Denominator;

	vec3 Ks = F;
	vec3 Kd = vec3(1.0) - Ks;
	Kd *= 1.0 - M;

	float NDotL = max(dot(N, L), 0.0);
	return (Kd * A / PI + Specular) * Ra * NDotL;
}

// ----------------------------------------------------------------------------

void main()
{
	vec3 FragPos	= texture(lGBuffer.Position, vTexCoords).rgb;
	vec3 N			= texture(lGBuffer.Normal, vTexCoords).rgb;
	vec3 Albedo		= texture(lGBuffer.AlbedoS, vTexCoords).rgb;
	float Roughness = texture(lGBuffer.RoughMetalAo, vTexCoords).r;
	float Metallic  = texture(lGBuffer.RoughMetalAo, vTexCoords).g;
	float AO		= texture(lGBuffer.RoughMetalAo, vTexCoords).b;

	vec4 FragPosLightSpace = uLightViewProjection * vec4(FragPos, 1.0);

	N = normalize(N);

	vec3 V = normalize(lViewpos - FragPos);
	vec3 R = reflect(-V, N);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, Albedo, Metallic);

	vec3 Lo = vec3(0.0);

	for(int i = 0; i < ldLightsActive; i++)
	{
		Lo += CalculateLo(-ldLights[i].Direction, N, V, ldLights[i].Diffuse, F0, Roughness, Metallic, Albedo);
	}

	for(int i = 0; i < lpLightsActive; i++)
	{
		vec3 L = normalize(lpLights[i].Position - FragPos);
		float distance = length(lpLights[i].Position - FragPos);
		float attenuation = 1.0/(distance * distance);
		vec3 Ra = lpLights[i].Diffuse * attenuation;
		Lo += CalculateLo(L, N, V, Ra, F0, Roughness, Metallic, Albedo);
	}
	
	vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, Roughness);

	vec3 Ks = F;
	vec3 Kd = 1.0 - Ks;
	Kd *= 1.0 - Metallic;
	
	vec3 Irradiance = texture(lIBL.IrradianceMap, N).rgb;
	vec3 Diffuse    = Irradiance * Albedo;

	const float MAX_REFLECTION_LOD = 4.0;
    vec3 PreFilteredColor = textureLod(lIBL.PreFilterMap, R, Roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 BRDF  = texture(lIBL.BRDFLookup, vec2(max(dot(N, V), 0.0), Roughness)).rg;
    vec3 Specular = PreFilteredColor * (F * BRDF.x + BRDF.y);

	vec3 Ambient = (Kd * Diffuse + Specular) * AO;
	vec3 Color = (Ambient + Lo) * CalculateShadows(FragPosLightSpace);

	// HDR tonemapping
    Color = Color / (Color + vec3(1.0));
    // gamma correct
    Color = pow(Color, vec3(1.0/2.2));

	lResult = vec4(Color, 1.0);
}