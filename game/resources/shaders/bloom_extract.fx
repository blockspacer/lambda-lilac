#include "common.fx"

struct VSOutput
{
  float4 position : SV_POSITION0;
  float2 tex      : TEX_COORD;
};

VSOutput VS(uint id: SV_VertexID)
{
  VSOutput vOut;
  vOut.tex = float2((id << 1) & 2, id & 2);
  vOut.position = float4(vOut.tex * float2(2, -2) + float2(-1, 1), 0, 1);
  return vOut;
}

Texture2D tex_albedo    : register(t0);

static const float3 l = float3(0.2126f, 0.7152f, 0.0722f);
static const float bloom_threshold = 1.0f;

float4 PS(VSOutput pIn) : SV_TARGET0
{
  float3 c = Sample(tex_albedo, SamLinearClamp, pIn.tex).xyz;
  float brightness = dot(c, l);
  return float4(lerp(0.0f, c, when_ge(brightness, bloom_threshold)), 1.0f);
}
