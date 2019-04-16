#include "../common.fxh"
#include "../pbr.fxh"

#if !defined(__SHADOW_MAPPING_PUBLISH__)
float calcShadow(float2 a, float b)
{
  return 0.0f;
}
#endif

Make_Texture2D(tex_shadow_map_01, 0);
Make_Texture2D(tex_shadow_map_02, 1);
Make_Texture2D(tex_shadow_map_03, 2);
Make_Texture2D(tex_overlay, 3);
Make_Texture2D(tex_position, 4);
Make_Texture2D(tex_normal, 5);
Make_Texture2D(tex_metallic_roughness, 6);

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

Make_CBuffer(cbUser, cbUserIdx)
{
  float4x4 light_view_projection_matrix_01;
  float4x4 light_view_projection_matrix_02;
  float4x4 light_view_projection_matrix_03;
  float3 light_camera_position_01;
  float3 light_camera_position_02;
  float3 light_camera_position_03;
  float light_far_01;
  float light_far_02;
  float light_far_03;
};

float linearizeOrtho(float depth)
{
  return depth * light_far_02;
}

float2 linearizeOrtho(float2 depth)
{
  return depth * light_far_02;
}

float3 linearizeOrtho(float3 depth)
{
  return depth * light_far_02;
}

float InFrustum(float4 position, float4x4 view_projection_matrix)
{
  float4 trans_position = mul(view_projection_matrix, position);
  trans_position.xyz /= trans_position.w;
  float2 coords = trans_position.xy * float2(0.5f, -0.5f) + 0.5f;
  return 1.0f - clamp(when_le(coords.x, 0.0f) + when_ge(coords.x, 1.0f) + when_le(coords.y, 0.0f) + when_ge(coords.y, 1.0f) + when_le(trans_position.z, 0.0f) + when_ge(trans_position.z, 1.0f), 0.0f, 1.0f);
}

float4 PS(VSOutput pIn) : SV_TARGET0
{
  float4 position = float4(Sample(tex_position, SamLinearClamp, pIn.tex).xyz, 1.0f);

  float far = 0.0f;
  float3 camera_position = 0.0f;
  float4x4 view_projection_matrix = 0.0f;


  if (InFrustum(position, light_view_projection_matrix_01))
  {
    far = light_far_01;
    camera_position = light_camera_position_01;
    view_projection_matrix = light_view_projection_matrix_01;
  }
  else if (InFrustum(position, light_view_projection_matrix_02))
  {
    far = light_far_02;
    camera_position = light_camera_position_02;
    view_projection_matrix = light_view_projection_matrix_02;
  }
  else if (InFrustum(position, light_view_projection_matrix_03))
  {
    far = light_far_03;
    camera_position = light_camera_position_03;
    view_projection_matrix = light_view_projection_matrix_03;
  }

  float4 trans_position = mul(view_projection_matrix, position);
  trans_position.xyz /= trans_position.w;
  float2 coords = trans_position.xy * float2(0.5f, -0.5f) + 0.5f;
  float position_depth = linearizeOrtho(trans_position.z * 0.5f + 0.5f);

  float4 overlay = tex_overlay.Sample(SamLinearClamp, coords);

  float hide_shadows = clamp(when_le(coords.x, 0.0f) + when_ge(coords.x, 1.0f) + when_le(coords.y, 0.0f) + when_ge(coords.y, 1.0f) + when_le(trans_position.z, 0.0f) + when_ge(trans_position.z, 1.0f), 0.0f, 1.0f);

  float2 shadow_map_depth = 0.0f;

  if (InFrustum(position, light_view_projection_matrix_01))
  {
    shadow_map_depth = tex_shadow_map_01.Sample(SamAnisotrophicClamp, coords).xy;
  }
  else if (InFrustum(position, light_view_projection_matrix_02))
  {
    shadow_map_depth = tex_shadow_map_02.Sample(SamAnisotrophicClamp, coords).xy;
  }
  else if (InFrustum(position, light_view_projection_matrix_03))
  {
    shadow_map_depth = tex_shadow_map_03.Sample(SamAnisotrophicClamp, coords).xy;
  }

  shadow_map_depth = linearizeOrtho(shadow_map_depth);
  
  float cs = calcShadow(shadow_map_depth, position_depth);

  float3 in_shadow = lerp(float3(cs, cs, cs) * (overlay.xyz * overlay.w), float3(1.0f, 1.0f, 1.0f), hide_shadows);

  float3 normal = normalize(Sample(tex_normal, SamLinearClamp, pIn.tex).rgb * 2.0f - 1.0f);

  float2 metallic_roughness = Sample(tex_metallic_roughness, SamLinearClamp, pIn.tex).rg;
  float  metallic  = metallic_roughness.r;
  float  roughness = metallic_roughness.g;

  float3 light = light_colour * in_shadow;

  float3 col;
  
  col = PBRDirectional(
    light_direction, camera_position,
    position.xyz, normal, metallic,
    roughness, light
  );

  return float4(col, 1.0f);
}
