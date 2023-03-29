#include "common.fxh"

struct VSInput
{
  float3 position : Positions;
};

struct VSOutput
{
  float4 position  : SV_POSITION0;
  float4 hPosition : H_POSITION;
};

VSOutput VS(VSInput vIn, uint instanceID : SV_InstanceID)
{
  VSOutput vOut;
  vOut.hPosition = mul(model_matrix[instanceID], float4(vIn.position, 1.0f));
  vOut.position  = mul(view_projection_matrix, vOut.hPosition);
  return vOut;
}

struct PSOutput
{
  float4 albedo   : SV_Target0;
};

PSOutput PS(VSOutput pIn)
{
  PSOutput pOut;
  pOut.albedo = 1.0f;
  return pOut;
}
