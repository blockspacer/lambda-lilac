#include "common.fx"

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
  float4 colour    : COLOUR;
  float2 tex       : TEX_COORD;
};

cbuffer cbPerMesh
{
  float4x4 model_matrix;
  float4x4 light_view_projection_matrix;
}

cbuffer cbPerPass
{
  float4 light_colour;
}

VSOutput VS(VSInput vIn)
{
  VSOutput vOut;
  vOut.hPosition = mul(model_matrix, float4(vIn.position, 1.0f));
  vOut.position  = mul(light_view_projection_matrix, vOut.hPosition);
  vOut.colour    = float4(1.0f, 1.0f, 1.0f, 1.0f);
  vOut.tex       = vIn.tex;
  vOut.normal    = normalize(mul((float3x3)model_matrix, vIn.normal));
  return vOut;
}

Texture2D tex_albedo   : register(t0);
Texture2D tex_normal   : register(t1);

struct PSOutput
{
  float4 shadow   : SV_Target0;
  float4 position : SV_Target1;
  float4 normal   : SV_Target2;
  float4 flux     : SV_Target3;
};

PSOutput PS(VSOutput pIn)
{
  float4 albedo = tex_albedo.Sample(SamLinearWarp, pIn.tex);
  float depth = pIn.position.z * 0.5f + 0.5f;

  if(albedo.a < 0.5f)
  {
    discard;
  }

  float dx = ddx(depth);
  float dy = ddy(depth);
  float moment2 = depth * depth + 0.25f * (dx * dx + dy * dy);
  
  PSOutput pOut;
  pOut.shadow   = float4(depth, moment2, depth, 1.0f);
  pOut.position = float4(pIn.hPosition.xyz, 1.0f);
  pOut.normal   = float4(pIn.normal * 0.5f + 0.5f, 1.0f);
  pOut.flux     = albedo * pIn.colour * light_colour;

  return pOut;
}