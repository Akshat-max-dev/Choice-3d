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
    sampler2D BRDFLookup;
};

uniform IBL lIBL;

struct PointLight
{
	vec3 Position;
	vec3 Diffuse;
	vec3 Specular;
	
	float Radius;
};

uniform PointLight lpLights[32];
uniform vec3 lViewpos;
uniform int lpLightsActive;

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
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}   
// ----------------------------------------------------------------------------
void main()
{
    vec3 Normal = normalize(texture(lGBuffer.Normal, vTexCoords).rgb);
    vec3 FragPos = texture(lGBuffer.Position, vTexCoords).rgb;
    vec3 Albedo = texture(lGBuffer.AlbedoS, vTexCoords).rgb;
    float Metallic = texture(lGBuffer.RoughMetalAo, vTexCoords).g;
    float Roughness = texture(lGBuffer.RoughMetalAo, vTexCoords).r;
    float AO = texture(lGBuffer.RoughMetalAo, vTexCoords).b;

    vec3 ViewDir = normalize(lViewpos - FragPos);
    vec3 R = reflect(-ViewDir, Normal);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, Albedo, Metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < lpLightsActive; i++)
    {
        // calculate per-light radiance
        vec3 L = normalize(lpLights[i].Position - FragPos);
        vec3 H = normalize(ViewDir + L);
        float distance = length(lpLights[i].Position - FragPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lpLights[i].Diffuse * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(Normal, H, Roughness);   
        float G   = GeometrySmith(Normal, ViewDir, L, Roughness);      
        vec3  F   = fresnelSchlick(max(dot(H, ViewDir), 0.0), F0);

        vec3 nominator    = NDF * G * F; 
        float denominator = 4 * max(dot(Normal, ViewDir), 0.0) * max(dot(Normal, L), 0.0) + 0.001;
        vec3 specular = nominator / denominator;
    
         // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - Metallic;	  

        // scale light by NdotL
        float NdotL = max(dot(Normal, L), 0.0);  

        // add to outgoing radiance Lo
        Lo += (kD * Albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }

    vec3 F = fresnelSchlickRoughness(max(dot(Normal, ViewDir), 0.0), F0, Roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - Metallic;

    vec3 irradiance   = texture(lIBL.IrradianceMap, Normal).rgb;
    vec3 diffuse      = irradiance * Albedo;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(lIBL.PreFilterMap, R,  Roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(lIBL.BRDFLookup, vec2(max(dot(Normal, ViewDir), 0.0), Roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * AO;
    
    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2));

    lResult = vec4(color, 1.0);
}