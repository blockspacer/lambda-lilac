#include "common.fxh"
#include "tbn.fxh"

struct VSInput
{
  float3 position : Positions;
  float4 colour   : Colours;
};

struct VSOutput
{
  float4 position  : SV_POSITION0;
  float4 hPosition : H_POSITION;
  float4 colour    : COLOUR;
};

Make_CBuffer(cbPerMesh, 0)
{
  float4x4 view_projection_matrix;
}

VSOutput VS(VSInput vIn)
{
  VSOutput vOut;
  vOut.hPosition = float4(vIn.position, 1.0f);
  vOut.position  = mul(view_projection_matrix, vOut.hPosition);
  vOut.colour    = vIn.colour;

  return vOut;
}

struct PSOutput
{
  float4 albedo   : SV_Target0;
};

Make_CBuffer(cbPerMesh, 1)
{
  float2 metallic_roughness;
  float3 camera_position;
}

PSOutput PS(VSOutput pIn)
{
  PSOutput pOut;
  pOut.albedo = pIn.colour;
  return pOut;
}