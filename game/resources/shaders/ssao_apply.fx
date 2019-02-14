#include "common.fx"

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

Texture2D tex_albedo    : register(t0);
Texture2D tex_ssao      : register(t1);

float4 PS(VSOutput pIn) : SV_TARGET0
{
  float4 albedo = Sample(tex_albedo, SamLinearClamp, pIn.tex);
  float  ao     = Sample(tex_ssao, SamLinearClamp,  pIn.tex).r;

  //if (pIn.tex.x < 0.5f)
    return Sample(tex_ssao, SamLinearClamp,  pIn.tex);
  //else
  //  return albedo;
}
