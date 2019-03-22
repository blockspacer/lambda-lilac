#include "common.fx"

struct VSInput
{
  float3 position : Positions;
  float2 tex      : TexCoords;
};

struct VSOutput
{
  float4 position  : SV_POSITION0;
  float4 hPosition : HPOSITION;
  float2 tex       : TEX_COORD;
};

cbuffer cbPerMesh
{
  float4x4 model_matrix;
  float4x4 light_view_projection_matrix;
}

cbuffer cbPostProcess
{
  float3 light_camera_position;
  float  light_far  = 10.0f;
}

VSOutput VS(VSInput vIn)
{
  VSOutput vOut;
  vOut.hPosition   = mul(model_matrix, float4(vIn.position, 1.0f));
  vOut.hPosition.x = -vOut.hPosition.x;
  vOut.position    = mul(light_view_projection_matrix, vOut.hPosition);
  vOut.tex         = vIn.tex;
  return vOut;
}

Texture2D tex_albedo   : register(t0);

// TODO (Hilze): Support alpha.
float4 PS(VSOutput pIn) : SV_Target0
{
  if (tex_albedo.Sample(SamLinearWarp, pIn.tex).a < 0.5f)
    discard;

  float4 position = pIn.hPosition;
  position.x = -position.x;
  float depth = length(position - light_camera_position) / light_far;

  float dx = ddx(depth);
  float dy = ddy(depth);
  float moment2 = depth * depth + 0.25f * (dx * dx + dy * dy);
  return float4(depth, moment2, depth, 1.0f);
}