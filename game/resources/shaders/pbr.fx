#include "common.fx"

struct PBRData
{
  float3 N;
  float3 V;
  float3 L;
  float attenuation;
  float metallic;
  float roughness;
  float3 colour;
};
float3 PBR(PBRData data);

float3 PBRSpot(float3 light_position, float3 camera_position,
  float3 world_position, float3 normal, float metallic, float roughness, 
  float3 light_colour, float3 ambient_colour, float radius,
  float3 direction, float cut_off, float outer_cut_off)
{

  PBRData pbr_data;
  pbr_data.N = normalize(normal);
  pbr_data.V = normalize(camera_position - world_position);
  pbr_data.L = normalize(light_position - world_position);
  pbr_data.metallic  = metallic;
  pbr_data.roughness = roughness;
  pbr_data.colour    = light_colour;

  float distance = clamp(length(light_position - world_position), 0.0f, radius);
  
  // Inv normalized squared.
  pbr_data.attenuation = saturate(1.0f - (distance / radius));
  pbr_data.attenuation *= pbr_data.attenuation;

  float co = cos(cut_off * 0.5f);
  float oco = cos(outer_cut_off * 0.5f);

  float theta = dot(pbr_data.L, normalize(-direction));
  float epsilon = co - oco;
  float intensity = clamp((theta - oco) / epsilon, 0.0f, 1.0f);
  
  pbr_data.attenuation *= intensity;
  pbr_data.attenuation = 1.0f;

  return PBR(pbr_data);
}

float3 PBRPoint(float3 light_position, float3 camera_position,
  float3 world_position, float3 normal, float metallic, float roughness,
  float3 light_colour, float3 ambient_colour, float radius)
{

  PBRData pbr_data;
  pbr_data.N = normalize(normal);
  pbr_data.V = normalize(camera_position - world_position);
  pbr_data.L = normalize(light_position  - world_position);
  pbr_data.metallic = metallic;
  pbr_data.roughness = roughness;
  pbr_data.colour    = light_colour;

  float distance = length(light_position - world_position);

  // Inv normalized squared.
  pbr_data.attenuation = saturate(1.0f - (distance - radius) / radius);
  pbr_data.attenuation *= pbr_data.attenuation;

  // Inv sqr distance.
  //pbr_data.attenuation = clamp(1.0f - (distance / radius) * (distance / radius), 0.0f, 1.0f);

  return PBR(pbr_data);
}

float3 PBRDirectional(float3 light_dir, float3 camera_position,
  float3 world_position, float3 normal, float metallic,
  float roughness, float3 light_colour)
{

  PBRData pbr_data;
  pbr_data.N = normalize(normal);
  pbr_data.V = normalize(camera_position - world_position);
  pbr_data.L = normalize(light_dir);
  pbr_data.attenuation = 1.0f;
  pbr_data.metallic    = metallic;
  pbr_data.roughness   = roughness;
  pbr_data.colour      = light_colour;

  return PBR(pbr_data);
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH * NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0f);
    float k = (r * r) / 8.0f;

    float num   = NdotV;
    float denom = NdotV * (1.0f - k) + k;
	
    return num / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0f);
    float NdotL = max(dot(N, L), 0.0f);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

#define VIOLET_GAMMA_CORRECT 0
float3 PBR(PBRData data)
{
  float3 Lo = 0.0f;

  float3 H = normalize(data.V + data.L);

  float3 F0 = lerp(0.04f, 1.0f, data.metallic);
  
  // Per light.
  if (length(data.colour) * data.attenuation <= 0.0f)
    return 0.0f;

  float3 radiance = data.colour * data.attenuation;

  float NDF = DistributionGGX(data.N, H, data.roughness);
  float G   = GeometrySmith(data.N, data.V, data.L, data.roughness);
  float3 F  = FresnelSchlick(max(dot(H, data.V), 0.0f), F0);

  float3 numerator  = NDF * G * F;
  float denominator = 4.0f * max(dot(data.N, data.V), 0.0f) * max(dot(data.N, data.L), 0.0f) + 0.001f; // 0.001 to prevent divide by zero.
  float3 specular   = numerator / denominator;

  float3 kS   = F;
  float3 kD   = 1.0f - kS;
  kD         *= 1.0f - data.metallic;
  float NdotL = max(dot(data.N, data.L), 0.0f);
  Lo          = (kD * 1.0f / PI + specular) * radiance * NdotL;

#if VIOLET_GAMMA_CORRECT
    Lo = Lo / (Lo + 1.0f);
    Lo = pow(Lo, 1.0 / 2.2f);  
#endif

  return Lo;
}