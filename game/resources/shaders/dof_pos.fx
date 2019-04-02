#include "common.fx"

Make_CBuffer(cbPerPass, 0)
{
  float3 camera_position;
  float camera_far;
  float delta_time;
};

struct VSOutput
{
  float4 position : SV_POSITION0;
  float2 tex      : TEX_COORD;
};

Make_Texture2D(tex_prev_pos, 0);
Make_Texture2D(tex_position, 1);

VSOutput VS(uint id: SV_VertexID)
{
  VSOutput vOut;
  vOut.tex = float2((id << 1) & 2, id & 2);
  vOut.position = float4(vOut.tex * float2(2, -2) + float2(-1, 1), 0, 1);
  return vOut;
}

static const float speed = 10000.0f;

float BezierBlend(float t)
{
    return (t * t) * (3.0f - 2.0f * t);
}

float4 PS(VSOutput pIn) : SV_TARGET0
{
  const float old_pos = tex_prev_pos[uint2(0,0)].x;
  const float new_pos = length(camera_position - Sample(tex_position, SamLinearClamp, 0.5f).xyz);

  const float spd = speed * delta_time;
  const float dir = clamp(old_pos - new_pos, -1.0f, 1.0f);
  const float dst = (old_pos - new_pos) / camera_far;
  const float fnl = dir * clamp(BezierBlend(dst), 0.0f, 1.0f) * spd; 



  float final_pos = old_pos - fnl;
  final_pos = clamp(final_pos, 0.0f, camera_far);

  return final_pos;
}
