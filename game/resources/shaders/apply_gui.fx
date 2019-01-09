#include "resources/shaders/common.fx"

struct VSInput
{
  float3 position : POSITION;
};

struct VSOutput
{
  float2 tex      : TEX_COORD;
  float4 position : SV_POSITION;
};

VSOutput VS(VSInput vIn, uint id : SV_VertexID)
{
  VSOutput vOut;
  vOut.tex      = float2((id << 1) & 2, id & 2);
  vOut.position = float4(vOut.tex * float2(2, -2) + float2(-1, 1), 0, 1);

  return vOut;
}

Texture2D    tex_albedo : register(t0);
Texture2D    tex_gui    : register(t1);

float4 PS(VSOutput pIn) : SV_TARGET0
{
  float4 gui = tex_gui.Sample(SamLinearClamp, pIn.tex);
  return lerp(tex_albedo.Sample(SamLinearClamp, pIn.tex), gui, gui.a);
}