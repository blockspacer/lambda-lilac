[DEFAULT|NO_DIRECTIONAL|ESM_DIRECTIONAL|VSM_DIRECTIONAL|NO_SPOT|ESM_SPOT|VSM_SPOT|NO_POINT|ESM_POINT|VSM_POINT]
#include "../common.fxh"
#include "../pbr.fxh"

// Get the light type.
#if TYPE == NO_DIRECTIONAL || TYPE == VSM_DIRECTIONAL || TYPE == ESM_DIRECTIONAL
#define LIGHT_DIRECTIONAL 1
#elif TYPE == NO_SPOT || TYPE == VSM_SPOT || TYPE == ESM_SPOT
#define LIGHT_SPOT 1
#elif TYPE == NO_POINT || TYPE == VSM_POINT || TYPE == ESM_POINT
#define LIGHT_POINT 1
#endif

// Get the shadow type.
#if TYPE == ESM_DIRECTIONAL || TYPE == ESM_SPOT || TYPE == ESM_POINT
#define ESM 1
#define VSM 0
#define NO_SHADOWS  0
#elif TYPE == VSM_DIRECTIONAL || TYPE == VSM_SPOT || TYPE == VSM_POINT
#define ESM 0
#define VSM 1
#define NO_SHADOWS  0
#elif TYPE == NO_DIRECTIONAL || TYPE == NO_SPOT || TYPE == NO_POINT || TYPE == DEFAULT
#define ESM 0
#define VSM 0
#define NO_SHADOWS  1
#endif

float linearStep(float min, float max, float val)
{
  return clamp((val - min) / (max - min), 0.0f, 1.0f);
}

#if VSM
float calcShadow(float2 moments, float compare)
{
  float p = step(compare, moments.x);
  float variance = max(moments.y - moments.x * moments.x, 0.001f);

  float d = (compare - moments.x);
  float p_max = linearStep(0.2f, 1.0f, variance / (variance + d * d));

  return min(max(p, p_max), 1.0f);
}
#elif ESM
static const float kESMMultiplier = 5.0f;

float calcShadow(float2 moments, float compare)
{
  const float val = clamp(exp((moments.x - compare) * kESMMultiplier), 0.0f, 1.0f);
  return linearStep(0.2f, 1.0f, val);
}
#else
float calcShadow(float2 moments, float compare)
{
  return 1.0f;
}
#endif

#if LIGHT_POINT
Make_TextureCube(tex_shadow_map, 0);
#elif !NO_SHADOWS
Make_Texture2D(tex_shadow_map, 0);
#endif
Make_Texture2D(tex_overlay, 1);
Make_Texture2D(tex_position, 2);
Make_Texture2D(tex_normal, 3);
Make_Texture2D(tex_metallic_roughness, 4);

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

Make_CBuffer(cbPostProcess, 0)
{
  float4x4 light_view_projection_matrix;
  float3 camera_position;
  float3 light_position;
  float3 light_direction;
  float3 light_colour;
  float  light_cut_off;
  float  light_outer_cut_off;
  float  light_near;
  float  light_far;
};

float4 PS(VSOutput pIn) : SV_TARGET0
{
  float4 position = float4(Sample(tex_position, SamLinearClamp, pIn.tex).xyz, 1.0f);

#if LIGHT_DIRECTIONAL && !NO_SHADOWS
  float4 trans_position = mul(light_view_projection_matrix, position);
  trans_position.xyz /= trans_position.w;
  float2 coords = trans_position.xy * float2(0.5f, -0.5f) + 0.5f;
  float position_depth = trans_position.z * light_far;
  float4 overlay = tex_overlay.Sample(SamLinearClamp, coords);
  float hide_shadows = clamp(when_le(coords.x, 0.0f) + when_ge(coords.x, 1.0f) + when_le(coords.y, 0.0f) + when_ge(coords.y, 1.0f) + when_le(trans_position.z, 0.0f) + when_ge(trans_position.z, 1.0f), 0.0f, 1.0f);

#elif LIGHT_POINT && !NO_SHADOWS
  float4 overlay = 1.0f;
  float3 coords = position.xyz - light_position;
  float position_depth = length(coords);
  coords = normalize(coords);
  float hide_shadows = position_depth > light_far;

#elif LIGHT_SPOT && !NO_SHADOWS

  float4 trans_position = mul(light_view_projection_matrix, position);
  trans_position.xyz /= trans_position.w;
  float2 coords = trans_position.xy * float2(0.5f, -0.5f) + 0.5f;
  float position_depth = length(position.xyz - light_position);
  float4 overlay = tex_overlay.Sample(SamLinearClamp, coords);
  float hide_shadows = clamp(when_le(coords.x, 0.0f) + when_ge(coords.x, 1.0f) + when_le(coords.y, 0.0f) + when_ge(coords.y, 1.0f) + when_le(trans_position.z, 0.0f) + when_ge(trans_position.z, 1.0f), 0.0f, 1.0f);
#endif

#if NO_SHADOWS
  float light_factor = 1.0f;
#else 
  float2 shadow_map_depth = tex_shadow_map.Sample(SamLinearClamp, coords).xy;
  float cs = calcShadow(shadow_map_depth, position_depth);

  if (cs <= 0.0f || hide_shadows >= 1.0f)
    return 0.0f;

  float3 light_factor = lerp(float3(cs, cs, cs) * (overlay.xyz * overlay.w), float3(1.0f, 1.0f, 1.0f), hide_shadows);
  
  if (cs <= 0.0f || hide_shadows >= 1.0f)
    return 0.0f;
#endif

  float3 normal = normalize(Sample(tex_normal, SamLinearClamp, pIn.tex).rgb * 2.0f - 1.0f);

  float2 metallic_roughness = Sample(tex_metallic_roughness, SamLinearClamp, pIn.tex).rg;
  float  metallic  = metallic_roughness.r;
  float  roughness = metallic_roughness.g;

  float3 light = light_colour * light_factor;

  float3 col = 0.0f;

#if LIGHT_DIRECTIONAL
  col = PBRDirectional(
    light_direction, camera_position,
    position.xyz, normal, metallic,
    roughness, light
  );
#elif LIGHT_POINT
  col = PBRPoint(
    light_position, light_position,
    position.xyz, normal, metallic, 
    roughness, light, light_far
  );
#elif LIGHT_SPOT
  col = PBRSpot(
    light_position, light_position,
    position.xyz, normal, metallic, roughness,
    light, light_far,
    light_direction, light_cut_off, light_outer_cut_off
  );
#else
  col = light;
#endif
  
  return float4(col, 1.0f);
}
