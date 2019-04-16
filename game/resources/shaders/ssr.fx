#include "common.fxh"

struct VSOutput
{
  float4 position : SV_POSITION;
  float2 tex      : TEXCOORD;
};

VSOutput VS(uint id: SV_VertexID)
{
  VSOutput vOut;
  vOut.tex      = float2((id << 1) & 2, id & 2);
  vOut.position = float4(vOut.tex * float2(2, -2) + float2(-1, 1), 0, 1);

  return vOut;
}

Make_Texture2D(tex_albedo, 0);
Make_Texture2D(tex_position, 1);
Make_Texture2D(tex_normal, 2);

float2 getTex(float3 world_pos)
{
  float4 tex = mul(view_projection_matrix, float4(world_pos, 1.0f));
  return (tex.xy / tex.w) * float2(0.5f, -0.5f) + 0.5f;
}

float4 PS(VSOutput pIn) : SV_Target0
{
  float3 position = Sample(tex_position, SamLinearClamp, pIn.tex).xyz;
  float3 normal = Sample(tex_normal, SamAnisotrophicClamp, pIn.tex).xyz * 2.0f - 1.0f;
  float2 tex = getTex(position);

  float4 reflected = Sample(tex_albedo, SamLinearClamp, tex.xy);
  return Sample(tex_albedo, SamLinearClamp, pIn.tex) * 0.5f + reflected * 0.5f;
}