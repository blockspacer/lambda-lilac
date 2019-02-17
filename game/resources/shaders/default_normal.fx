#include "tbn.fx"

struct VSInput
{
  float3 position : Positions;
  float3 normal   : Normals;
  float2 tex      : TexCoords;
};

struct VSOutput
{
  float4 position  : SV_POSITION0;
  float4 hPosition : H_POSITION;
  float3 normal    : NORMAL;
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
  vOut.hPosition = mul(model_matrix, float4(vIn.position, 1.0f));
  vOut.position  = mul(view_projection_matrix, vOut.hPosition);
  vOut.normal    = normalize(mul((float3x3)model_matrix, vIn.normal));
  vOut.tex       = vIn.tex;
  return vOut;
}

Texture2D tex_albedo   : register(t0);
Texture2D tex_normal   : register(t1);
SamplerState sam_point : register(s0);

float4 PS(VSOutput pIn) : SV_Target0
{
  if (tex_albedo.Sample(sam_point, pIn.tex).a < 0.25f)
  {
    discard;
  }

  float3 N = normalize(pIn.normal);

  // TODO (Hilze): Re-add normal mapping.
  //float3 MN = (tex_normal.Sample(sam_point, pIn.tex).xyz) * 2.0f - 1.0f;
  //float3 V = normalize(pIn.hPosition.xyz - camera_position);
  //N = perturb_normal(N, MN, V, pIn.tex);

  // TODO (Hilze): Add alpha discard.

  return float4(N * 0.5f + 0.5f, 1.0f);
}
