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

float4 PS(VSOutput pIn) : SV_TARGET0
{
    float3 c = Sample(tex_albedo, SamLinearClamp, pIn.tex).rgb;
    float luminosity = sqrt(0.299f * (c.r * c.r) + 0.587f * (c.g * c.g) + 0.114f * (c.b * c.b));
    return float4(luminosity, 1.0f, 1.0f, 1.0f);
}
