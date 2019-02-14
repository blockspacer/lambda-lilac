struct VSOutput
{
  float4 position : SV_POSITION0;
  float2 tex      : TEX_COORD;
};

VSOutput VS(uint id: SV_VertexID)
{
  VSOutput vOut;
  vOut.tex      = float2((id << 1)& 2, id& 2);
	vOut.position = float4(vOut.tex * float2(2, -2) + float2(-1, 1), 0, 1);
  return vOut;
}

cbuffer cbDynamicResolution
{
  float dynamic_resolution_scale;
};

Texture2D tex_to_screen : register(t0);
SamplerState sam_point  : register(s0);

float4 PS(VSOutput pIn) : SV_TARGET0
{
    return tex_to_screen.Sample(sam_point, pIn.tex * dynamic_resolution_scale);
}