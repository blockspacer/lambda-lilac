#include "common.fx"

struct VSOutput
{
  float4 position : SV_POSITION0;
  float2 tex      : TEX_COORD;
};

VSOutput VS(uint id: SV_VertexID)
{
  VSOutput vOut;
  vOut.tex = float2((id << 1) & 2, id & 2);
  vOut.position = float4(vOut.tex * float2(2, -2) + float2(-1, 1), 0, 1);
  return vOut;
}

Make_Texture2D(tex_albedo, 0);
Make_Texture2D(tex_luminosity, 1);

float4 PS(VSOutput pIn) : SV_TARGET0
{
  float3 albedo = Sample(tex_albedo, SamLinearClamp, pIn.tex).rgb;
  float1 luminosity = tex_luminosity.SampleLevel(SamLinearClamp, 0.0f, 100).r + tex_luminosity.SampleLevel(SamLinearClamp, 1.0f, 100).r;

  float key_value = 0.4f;
  float lux_min = 0.1f;
  float lux_max = 1.0f;
  float exposure = key_value / clamp(luminosity, lux_min, lux_max);

  return float4(albedo * exposure, 1.0f);
}
