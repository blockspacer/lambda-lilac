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

#define copy_resolution_scale user_data[0].x

float4 PS(VSOutput pIn) : SV_TARGET0
{
  return float4(pIn.tex, 0.0f, 1.0f);
  //return tex_to_screen.Sample(SamPointClamp, pIn.tex * copy_resolution_scale);
}