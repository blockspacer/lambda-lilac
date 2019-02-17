#include "common.fx"
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

#define VIOLET_GRID_ALBEDO 0
#define VIOLET_DYNAMIC_GRID_SIZE 0

PSOutput PS(VSOutput pIn)
{
  PSOutput pOut;
  pOut.albedo = tex_albedo.Sample(SamLinearWarp, pIn.tex) * pIn.colour;
  if (tex_albedo.Sample(SamLinearWarp, pIn.tex).a < 0.25f)
    discard;
  pOut.albedo.a = 1.0f;

#if VIOLET_GRID_ALBEDO
#if VIOLET_DYNAMIC_GRID_SIZE
  const float scale = PI * ((length(pIn.hPosition - camera_position) < 100.0f) ? (length(pIn.hPosition - camera_position) < 10.0f) ? 10.0f : 1.0f : 0.1f);
#else
  const float scale = PI;
#endif
  const float3 pl = pIn.hPosition * scale;
  const float3 ps = pIn.hPosition * (scale * 10.0f);

  float el = sin(pl.x) * sin(pl.y) * sin(pl.z);
  float es = sin(ps.x) * sin(ps.y) * sin(ps.z);

	pOut.albedo.rgb = lerp(lerp(0.4, 0.5, when_ge(es, 0.0f)), lerp(0.9, 1.0, when_ge(es, 0.0f)), when_ge(el, 0.0f));
#endif

  float3 N      = normalize(pIn.normal);
  float2 mr     = tex_mr.Sample(SamLinearWarp, pIn.tex).bg; //* metallic_roughness;
  pOut.position = float4(pIn.hPosition.xyz, 1.0f);
  pOut.normal   = float4(N * 0.5f + 0.5f, 1.0f);
  pOut.mr       = float4(mr, 0.0f, 1.0f);

  return pOut;
}
