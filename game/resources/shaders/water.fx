#include "common.fx"

struct VSOutput
{
  float4 position : SV_POSITION0;
  float2 tex      : TEX_COORD;
};

VSOutput VS(uint id: SV_VertexID)
{
  VSOutput vOut;
  vOut.tex      = float2((id << 1) & 2, id & 2);
  vOut.position = float4(vOut.tex * float2(2, -2) + float2(-1, 1), 0, 1);
  return vOut;
}

cbuffer CBUFFER
{
  float total_time;
  float water_height;
  float3 camera_position;
}

Texture2D tex_albedo    : register(t0);

static const float2 sin_intensity = float2(0.025f, 0.01f);
static const float2 speed = float2(3.0f, 1.25f);
static const float range = 2.0f;
static const float radius = 0.75f;
static const float softness = 0.85f;

float4 PS(VSOutput pIn) : SV_TARGET0
{
  float intensity = clamp(abs(camera_position.y - water_height) / range, 0.0f, 1.0f);
  float color_intensity_r = lerp(0.6f, 1.0f, 1.0f - intensity);
  float color_intensity_g = lerp(0.8f, 1.0f, 1.0f - intensity);

  float2 offset = float2(sin((total_time + pIn.tex.x) * speed.x), cos((total_time + pIn.tex.y)) * speed.y) * sin_intensity * intensity;
  float2 tex = (pIn.tex + offset) * (1.0f - sin_intensity * intensity * 2.0f) + sin_intensity * intensity;
  float4 raw_albedo = Sample(tex_albedo, SamLinearClamp, tex);
  float3 tone_mapped = raw_albedo.rgb * float3(color_intensity_r, color_intensity_g, 1.0f);
  
  // Vignette.
  float l = length(pIn.tex - 0.5f);
  float vignette = smoothstep(radius, radius - softness, l);
  tone_mapped = lerp(tone_mapped, tone_mapped * vignette, intensity * 0.666f);

  return float4(tone_mapped, raw_albedo.a);
}