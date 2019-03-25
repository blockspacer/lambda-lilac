#include "common.fx"
#include "pbr.fx"
#include "vsm_publish.fx"

struct VSOutput
{
  float2 tex      : TEX_COORD;
  float4 position : SV_POSITION;
};

VSOutput VS(uint id : SV_VertexID)
{
  VSOutput vOut;
  vOut.tex = float2((id << 1) & 2, id & 2);
  vOut.position = float4(vOut.tex * float2(2, -2) + float2(-1, 1), 0, 1);

  return vOut;
}

cbuffer cbPostProcess
{
  float4x4 light_view_projection_matrix;
  float3 light_camera_position;
  float3 light_position;
  float3 light_colour;
  float3 light_ambient;
  float  light_type;
  float  light_near = 0.001f;
  float  light_far  = 10.0f;
};

TextureCube tex_shadow_map   : register(t0);
Texture2D tex_overlay      : register(t1);
Texture2D tex_position     : register(t2);
Texture2D tex_normal       : register(t3);
Texture2D tex_metallic_roughness : register(t4);

float4 PS(VSOutput pIn) : SV_TARGET0
{
  float4 position = float4(Sample(tex_position, SamLinearClamp, pIn.tex).xyz, 1.0f);

  float3 dir = position.xyz - light_camera_position;
  float position_depth = length(dir);
  dir = normalize(dir);

  float3 shadow_map_depth = tex_shadow_map.Sample(SamAnisotrophicClamp, dir).xyz;

  float cs = calcShadow(shadow_map_depth, position_depth);

  if (position_depth > light_far || cs <= 0.0f)
    return 0.0f;

  float3 normal = normalize(Sample(tex_normal, SamLinearClamp, pIn.tex).rgb * 2.0f - 1.0f);

  float2 metallic_roughness = Sample(tex_metallic_roughness, SamLinearClamp, pIn.tex).rg;
  float  metallic  = metallic_roughness.r;
  float  roughness = metallic_roughness.g;

  float3 light = light_colour * cs;

  float3 col;

  col = PBRPoint(
    light_position, light_camera_position,
    position.xyz, normal, metallic, roughness,
    light, light_ambient, light_far
  );

  return float4(col, 1.0f);
}
