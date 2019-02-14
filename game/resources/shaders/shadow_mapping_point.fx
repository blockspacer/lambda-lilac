#include "common.fx"
#include "pbr.fx"

// TODO (Hilze): Remove ASAP!
#include "vsm_publish.fx"

struct VSInput
{
  float3 position : POSITION;
};

struct VSOutput
{
  float2 tex      : TEX_COORD;
  float4 position : SV_POSITION;
};

VSOutput VS(VSInput vIn, uint id : SV_VertexID)
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

Texture2D tex_shadow_map   : register(t0);
Texture2D tex_overlay      : register(t1);
Texture2D tex_position     : register(t2);
Texture2D tex_normal       : register(t3);
Texture2D tex_metallic_roughness : register(t4);

#define LINEARIZE(n, f, d) (2.0f * f * n / (f + n - (f - n) * d))

float linearizePerspective(float depth)
{
  return LINEARIZE(light_near, light_far, depth);
}

float2 linearizePerspective(float2 depth)
{
  return LINEARIZE(light_near, light_far, depth);
}

float3 linearizePerspective(float3 depth)
{
  return LINEARIZE(light_near, light_far, depth);
}

float4 PS(VSOutput pIn) : SV_TARGET0
{
  float4 position = float4(Sample(tex_position, SamLinearClamp, pIn.tex).xyz, 1.0f);

#ifdef CAST_SHADOWS
  float4 trans_position = mul(light_view_projection_matrix, position);
  trans_position.xyz /= trans_position.w;
  float2 coords = trans_position.xy * float2(0.5f, -0.5f) + 0.5f;
  float position_depth = linearizePerspective(trans_position.z * 0.5f + 0.5f);

  float4 overlay = tex_overlay.Sample(SamLinearClamp, coords);

  float hide_shadows = clamp(when_le(coords.x, 0.0f) + when_ge(coords.x, 1.0f) + when_le(coords.y, 0.0f) + when_ge(coords.y, 1.0f) + when_le(trans_position.z, 0.0f) + when_ge(trans_position.z, 1.0f), 0.0f, 1.0f);

  float3 shadow_map_depth = linearizePerspective(tex_shadow_map.Sample(SamAnisotrophicClamp, coords).xyz);

  float cs = calcShadow(shadow_map_depth, position_depth, light_far);

  float3 in_shadow = lerp(float3(cs, cs, cs) * (overlay.xyz * overlay.w), float3(1.0f, 1.0f, 1.0f), hide_shadows);
#else
  float in_shadow = 1.0f;
#endif

  float3 normal = normalize(Sample(tex_normal, SamLinearClamp, pIn.tex).rgb * 2.0f - 1.0f);

  float2 metallic_roughness = Sample(tex_metallic_roughness, SamLinearClamp, pIn.tex).rg;
  float  metallic  = metallic_roughness.r;
  float  roughness = metallic_roughness.g;

  float3 light = light_colour * in_shadow;

  float3 col;
  
  col = PBRPoint(
    light_position, light_camera_position,
    position.xyz, normal, metallic, roughness,
    light, light_ambient, light_far
  );

  return float4(col, 1.0f);
}
