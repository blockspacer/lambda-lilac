#include "common.fx"

Make_CBuffer(cbPerPass, 0)
{
  float4x4 view_matrix;
  float3 camera_position;
  float camera_far;
};

struct VSOutput
{
  float4 position : SV_POSITION0;
  float2 tex      : TEX_COORD;
};

Make_Texture2D(tex_albedo, 0);
Make_Texture2D(tex_blurred, 1);
Make_Texture2D(tex_dof_pos, 2);
Make_Texture2D(tex_position, 3);

static float distance = 1.0f / 15.0f;

float BezierBlend(float t)
{
    return (t * t) * (3.0f - 2.0f * t);
}

VSOutput VS(uint id: SV_VertexID)
{
  VSOutput vOut;
  vOut.tex = float2((id << 1) & 2, id & 2);
  vOut.position = float4(vOut.tex * float2(2, -2) + float2(-1, 1), 0, 1);
  return vOut;
}

float4 PS(VSOutput pIn) : SV_TARGET0
{
  float3 albedo    = Sample(tex_albedo, SamLinearClamp, pIn.tex).rgb;
  float3 blurred   = Sample(tex_blurred, SamLinearClamp, pIn.tex).rgb;

  float depth1 = tex_dof_pos[uint2(0,0)].r;
  //float depth2 = clamp(length(camera_position - tex_position.Sample(SamLinearClamp, pIn.tex).rgb), 0.0f, camera_far);

  float3 normal = normalize(view_matrix[2].xyz);
  float3 delta = Sample(tex_position, SamLinearClamp, pIn.tex).rgb - camera_position;
  float depth2 = clamp(abs(dot(dot(delta, normal) * normal, normal)), 0.0f, camera_far);
  
  float diff = clamp(-(depth1 - depth2) * distance, 0.0f, 1.0f);

  return float4(lerp(albedo, blurred, BezierBlend(diff)), 1.0f);
}
