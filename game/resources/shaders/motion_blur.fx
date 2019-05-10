#include "common.fxh"

struct VSOutput
{
  float4 position : SV_POSITION;
  float2 tex      : TEXCOORD;
};

VSOutput VS(uint id: SV_VertexID)
{
  VSOutput vOut;
  vOut.tex = float2((id << 1) & 2, id & 2);
  vOut.position = float4(vOut.tex * float2(2, -2) + float2(-1, 1), 0, 1);
  return vOut;
}

Make_Texture2D(tex_albedo, 0);
Make_Texture2D(tex_prev_albedo, 1);

float4 PS(VSOutput pIn) : SV_TARGET
{
  return Sample(tex_albedo, SamLinearClamp, pIn.tex) * 0.5 + Sample(tex_prev_albedo, SamLinearClamp, pIn.tex) * 0.5;
}
