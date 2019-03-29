struct VSInput
{
  float3 position : POSITIONS;
  float3 normal   : NORMALS;
  float2 tex      : TEX_COORDS;
};

struct VSOutput
{
  float4 position : SV_POSITION0;
  float3 normal   : NORMAL;
  float2 tex      : TEX_COORD;
};

cbuffer UniformBufferObject
{
  float4x4 model;
  float4x4 view;
  float4x4 proj;
};

VSOutput VS(VSInput pIn)
{
  VSOutput vOut;
  vOut.normal = pIn.normal;
  vOut.tex = pIn.tex;
  vOut.position = mul(mul(mul(float4(pIn.position, 1.0f), model), view), proj);
  return vOut;
}

SamplerState sam;
Texture2D    tex;

float4 PS(VSOutput pIn) : SV_TARGET0
{
  return tex.Sample(sam, pIn.tex);
}