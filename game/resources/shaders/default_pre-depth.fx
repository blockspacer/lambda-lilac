#include "resources/shaders/common.fx"

struct VSInput
{
  float3 position : POSITION;
  float2 tex      : TEX_COORD;
};

struct VSOutput
{
  float4 position  : SV_POSITION0;
  float2 tex       : TEX_COORD;
};

cbuffer cbPerMesh
{
  float4x4 model_matrix;
  float4x4 view_projection_matrix;
}

VSOutput VS(VSInput vIn)
{
  VSOutput vOut;
  float4 hPosition = mul(model_matrix, float4(vIn.position, 1.0f));
  vOut.position    = mul(view_projection_matrix, hPosition);
  vOut.tex         = vIn.tex;
  //vOut.position.z  = 1.0f - vOut.position.z / vOut.position.w;
  //vOut.position.w  = 1.0f;
  return vOut;
}

Texture2D tex_albedo   : register(t0);

void PS(VSOutput pIn)
{
  if (tex_albedo.Sample(SamLinearWarp, pIn.tex).a < 0.25f)
    discard;
}