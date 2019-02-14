#include "common.fx"

///////////////////////////////////////////////////////////////////////////
// Functions //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max((1.0f - roughness), F0) - F0) * pow(1.0f - cosTheta, 5.0f);
}

///////////////////////////////////////////////////////////////////////////
// Inputs /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
Texture2D tex_albedo      : register(t0);
Texture2D tex_position    : register(t1);
Texture2D tex_normal      : register(t2);
Texture2D tex_metallic_roughness : register(t3);

Texture2D tex_light_map   : register(t4);

Texture2D tex_irradiance  : register(t5);
Texture2D tex_prefiltered : register(t6);
Texture2D tex_brdfLUT     : register(t7);

cbuffer AmbientValues
{
  float ambient_intensity = 1.0f;
  float3 camera_position;
}

///////////////////////////////////////////////////////////////////////////
// Vertex /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
struct VSOutput
{
  float4 position : SV_POSITION0;
  float2 tex      : TEX_COORD;
};

VSOutput VS(uint id: SV_VertexID)
{
  VSOutput vOut;
  vOut.tex      = float2((id << 1) & 2, id & 2);
  vOut.position = float4(vOut.tex * float2(2, -2) + float2(-1, 1), 0, 1);
  return vOut;
}

///////////////////////////////////////////////////////////////////////////
// Pixel //////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
float4 PS(VSOutput pIn) : SV_TARGET0
{
  // Ambient.
  float3 N = normalize(Sample(tex_normal, SamLinearClamp, pIn.tex).rgb * 2.0f - 1.0f);
  float3 V = normalize(camera_position - Sample(tex_position, SamLinearClamp, pIn.tex).rgb);
  float3 R = reflect(-V, N);
  float2 metallic_roughness = Sample(tex_metallic_roughness, SamLinearClamp, pIn.tex).rg;
  float  metallic  = metallic_roughness.r;
  float  roughness = metallic_roughness.g;
  float3 F0 = lerp(0.04f, 1.0f, metallic);

  float ao  = 1.0f;
  float3 F  = FresnelSchlickRoughness(max(dot(N, V), 0.0f), F0, roughness);

  float3 kS = F;
  float3 kD = 1.0f - kS;
  kD       *= 1.0f - metallic;

  float3 diffuse = tex_irradiance.Sample(SamLinearClamp, SampleSphericalMap(N)).rgb * 1.0f;

  const float MAX_REFLECTION_LOD = 4.0f;
  float3 prefiltered = tex_prefiltered.SampleLevel(SamLinearClamp, SampleSphericalMap(R), roughness * MAX_REFLECTION_LOD).rgb;
  float2 brdf = tex_brdfLUT.Sample(SamLinearClamp, float2(max(dot(N, V), 0.0f), roughness)).rg;
  float3 specular = prefiltered * (F * brdf.x + brdf.y);

  float3 ambient = (kD * diffuse + specular) * ao * ambient_intensity;
  
#if VIOLET_GAMMA_CORRECT
  ambient = ambient / (ambient + 1.0f);
  ambient = pow(ambient, 1.0f / 2.2f);
#endif

  // Lighting.
  float4 albedo = Sample(tex_albedo, SamLinearClamp, pIn.tex);
  float3 light  = Sample(tex_light_map, SamLinearClamp, pIn.tex).rgb;

  return float4(pow(abs(albedo.rgb), 2.2f) * (light + ambient), albedo.a);
}
