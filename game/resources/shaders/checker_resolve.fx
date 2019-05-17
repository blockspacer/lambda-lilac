#include "common.fxh"

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

Make_Texture2D(tex1, 0);
Make_Texture2D(tex2, 1);

float4 PS(VSOutput pIn) : SV_TARGET0
{
    return Sample(tex1, SamPointClamp, pIn.tex) + Sample(tex2, SamPointClamp, pIn.tex);
}
