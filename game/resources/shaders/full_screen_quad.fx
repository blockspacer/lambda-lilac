#include "common.fxh"

struct VSOutput
{
  float4 position : SV_POSITION0;
  float2 tex      : TEX_COORD;
};

VSOutput VS(uint id: SV_VertexID)
{
  VSOutput vOut;
  vOut.tex      = float2((id << 1)& 2, id& 2);
	vOut.position = float4(vOut.tex * float2(2, -2) + float2(-1, 1), 0, 1);
  return vOut;
}

Make_Texture2D(tex_to_screen, 0);

Make_CBuffer(cbUser, cbUserIdx)
{
  float copy_resolution_scale;
}

float4 PS(VSOutput pIn) : SV_TARGET0
{
  return tex_to_screen.Sample(SamPointClamp, pIn.tex * copy_resolution_scale);
}