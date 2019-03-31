#include "common.fx"

struct VSOutput
{
  float2 tex      : TEX_COORD;
  float4 position : SV_POSITION;
};

VSOutput VS(uint id : SV_VertexID)
{
  VSOutput vOut;
  vOut.tex      = float2((id << 1) & 2, id & 2);
  vOut.position = float4(vOut.tex * float2(2, -2) + float2(-1, 1), 0, 1);

  return vOut;
}

Make_CBuffer(cbPostProcess, 0)
{
  float2 inv_texture_size;
  float2 blur_scale = float2(0.0f, 0.0f);
};

Make_Texture2D(tex_to_blur, 0);


static const float blur_table[5] = {
  0.016216f,
  0.054054f, 
  0.1216216f, 
  0.1945946f, 
  0.227027f
};

float4 PS(VSOutput pIn) : SV_TARGET0
{
  float4 colour = 0.0f;

  float2 scale = blur_scale * inv_texture_size;

  colour += Sample(tex_to_blur, SamLinearClamp, pIn.tex + -4.0f * scale) * blur_table[0];
  colour += Sample(tex_to_blur, SamLinearClamp, pIn.tex + -3.0f * scale) * blur_table[1];
  colour += Sample(tex_to_blur, SamLinearClamp, pIn.tex + -2.0f * scale) * blur_table[2];
  colour += Sample(tex_to_blur, SamLinearClamp, pIn.tex + -1.0f * scale) * blur_table[3];
  colour += Sample(tex_to_blur, SamLinearClamp, pIn.tex + +0.0f * scale) * blur_table[4];
  colour += Sample(tex_to_blur, SamLinearClamp, pIn.tex + +1.0f * scale) * blur_table[3];
  colour += Sample(tex_to_blur, SamLinearClamp, pIn.tex + +2.0f * scale) * blur_table[2];
  colour += Sample(tex_to_blur, SamLinearClamp, pIn.tex + +3.0f * scale) * blur_table[1];
  colour += Sample(tex_to_blur, SamLinearClamp, pIn.tex + +4.0f * scale) * blur_table[0];

  return colour;
}