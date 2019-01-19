#pragma once

///////////////////////////////////////////////////////////////////////////////
char kDefaultShader[] = R"(
#include "resources/shaders/common.fx"
#include "resources/shaders/tbn.fx"

struct VSInput
{
  float3 position : POSITION;
  float3 normal   : NORMAL;
  float2 tex      : TEX_COORD;
};

struct VSOutput
{
  float4 position  : SV_POSITION0;
  float4 hPosition : H_POSITION;
  float4 colour    : COLOUR;
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
  vOut.colour    = float4(1.0f, 1.0f, 1.0f, 1.0f);
  vOut.normal    = normalize(mul((float3x3)model_matrix, vIn.normal));
  vOut.tex       = vIn.tex;
  //vOut.position.z = 1.0f - vOut.position.z / vOut.position.w;
  //vOut.position.w = 1.0f;
  return vOut;
}

Texture2D tex_albedo   : register(t0);
Texture2D tex_normal   : register(t1);
Texture2D tex_mr       : register(t2);

struct PSOutput
{
  float4 albedo   : SV_Target0;
  float4 position : SV_Target1;
  float4 normal   : SV_Target2;
  float4 mr       : SV_Target3; // metallic roughness.
};

cbuffer cbPerMesh
{
  float2 metallic_roughness;
  float3 camera_position;
}


PSOutput PS(VSOutput pIn)
{
  PSOutput pOut;
  if (tex_albedo.Sample(SamLinearWarp, pIn.tex).a < 0.25f)
    discard;
  pOut.albedo.a = 1.0f;

	float ex = fmod(floor(abs(pIn.hPosition.x)), 2.0f);
	float ey = fmod(floor(abs(pIn.hPosition.y)), 2.0f);
	float ez = fmod(floor(abs(pIn.hPosition.z)), 2.0f);
	float sex = fmod(floor(abs(pIn.hPosition.x * 10.0f)), 2.0f);
	float sey = fmod(floor(abs(pIn.hPosition.y * 10.0f)), 2.0f);
	float sez = fmod(floor(abs(pIn.hPosition.z * 10.0f)), 2.0f);

	float even = fmod(fmod(ey + ez, 2.0f) + ex, 2.0f);
	float sub_even = fmod(fmod(sey + sez, 2.0f) + sex, 2.0f);
	
	pOut.albedo.rgb = lerp(lerp(0.4, 0.5, sub_even), lerp(0.9, 1.0, sub_even), even);

  float3 N      = normalize(pIn.normal);
  float2 mr     = tex_mr.Sample(SamLinearWarp, pIn.tex).bg; //* metallic_roughness;
  pOut.position = float4(pIn.hPosition.xyz, 1.0f);
  pOut.normal   = float4(N * 0.5f + 0.5f, 1.0f);
  pOut.mr       = float4(mr, 0.0f, 1.0f);

  return pOut;
}
)";
