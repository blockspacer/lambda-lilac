#include "resources/shaders/common.fx"

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
  vOut.tex      = float2((id << 1) & 2, id & 2);
  vOut.position = float4(vOut.tex * float2(2, -2) + float2(-1, 1), 0, 1);

  return vOut;
}

cbuffer cbPostProcess
{
  float2 inv_texture_size;
  float2 blur_scale = float2(0.0f, 0.0f);
};

Texture2D tex_to_blur  : register(t0);


static const float blur_table[4] = {
  1.0f / 64.0f,
  6.0f / 64.0f,
  15.0f / 64.0f,
  20.0f / 64.0f,
};

float4 blur(float4 c, float4 bc, float w)
{
  return float4(c.xy + bc.xy * w, 0.0f, 1.0f);
}

float4 PS(VSOutput pIn) : SV_TARGET0
{
  float4 colour = 0.0f;

  float2 scale = blur_scale * inv_texture_size;

  colour = blur(colour, tex_to_blur.Sample(SamLinearClamp, pIn.tex - 3.0f * scale), blur_table[0]);
  colour = blur(colour, tex_to_blur.Sample(SamLinearClamp, pIn.tex - 2.0f * scale), blur_table[1]);
  colour = blur(colour, tex_to_blur.Sample(SamLinearClamp, pIn.tex - 1.0f * scale), blur_table[2]);
  colour = blur(colour, tex_to_blur.Sample(SamLinearClamp, pIn.tex + 0.0f * scale), blur_table[3]);
  colour = blur(colour, tex_to_blur.Sample(SamLinearClamp, pIn.tex + 1.0f * scale), blur_table[2]);
  colour = blur(colour, tex_to_blur.Sample(SamLinearClamp, pIn.tex + 2.0f * scale), blur_table[1]);
  colour = blur(colour, tex_to_blur.Sample(SamLinearClamp, pIn.tex + 3.0f * scale), blur_table[0]);

  colour.b = tex_to_blur.Sample(SamLinearClamp, pIn.tex).b;

  return colour;
}