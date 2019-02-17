#include "common.fx"

struct VSInput
{
  float3 position : Positions;
  float2 tex      : TexCoords;
};

struct VSOutput
{
  float4 position  : SV_POSITION0;
  float2 tex       : TEX_COORD;
};

cbuffer cbPerMesh
{
  float4x4 model_matrix;
  float4x4 light_view_projection_matrix;
}

VSOutput VS(VSInput vIn)
{
  VSOutput vOut;
  float4 hPosition = mul(model_matrix, float4(vIn.position, 1.0f));
  vOut.position    = mul(light_view_projection_matrix, hPosition);
  vOut.tex         = vIn.tex;
  return vOut;
}

Texture2D tex_albedo   : register(t0);

// TODO (Hilze): Support alpha.
float4 PS(VSOutput pIn) : SV_Target0
{
  float alpha = tex_albedo.Sample(SamLinearWarp, pIn.tex).a;
  float depth = pIn.position.z * 0.5f + 0.5f;

  if(alpha < 0.5f)
  {
    discard;
  }

  float dx = ddx(depth);
  float dy = ddy(depth);
  float moment2 = depth * depth + 0.25f * (dx * dx + dy * dy);
  return float4(depth, moment2, depth, alpha);
}