#include "common.fx"
#include "sh.fx"

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
Texture2D tex_mra         : register(t3);

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

static const float3 g_coefficients[9] = {
  float3(+0.690826f, +0.791972f, +1.156998f),
  float3(-0.032214f, -0.032154f, -0.031143f),
  float3(+0.181760f, +0.213785f, +0.350721f),
  float3(+0.174773f, +0.165067f, +0.134813f),
  float3(-0.140021f, -0.133054f, -0.109773f),
  float3(+0.003920f, +0.002716f, -0.001993f),
  float3(-0.285459f, -0.332236f, -0.476629f),
  float3(-0.021688f, -0.022507f, -0.025049f),
  float3(+0.056419f, +0.054559f, +0.047959f)
};

static const float3 g_transfer_func = 1.0f;

///////////////////////////////////////////////////////////////////////////
// Pixel //////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
float4 PS(VSOutput pIn) : SV_TARGET0
{
  // Ambient.
  float3 N = normalize(Sample(tex_normal, SamLinearClamp, pIn.tex).rgb * 2.0f - 1.0f);
  float3 V = normalize(camera_position - Sample(tex_position, SamLinearClamp, pIn.tex).rgb);
  float3 R = reflect(-V, N);
  float3 mra = Sample(tex_mra, SamLinearClamp, pIn.tex).rgb;
  float  metallic  = mra.r;
  float  roughness = mra.g;
  float  ao        = mra.b;
  float3 F0 = lerp(0.04f, 1.0f, metallic);

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

  //ambient.rgb = EvalSH(N, g_coefficients, g_transfer_func);
  //light   = 1.0f; // Get rid of light so we can debug the SH.
  //ambient = 0.0f;

  return float4(pow(abs(albedo.rgb), 2.2f) * (light + ambient), albedo.a);
}
