#include "common.fx"

cbuffer buffer0 : register(b0)
{
	float4x4 projection_matrix;
};

Make_Texture2D(tex, 0);
Make_SamplerState(sam, 0);

struct VS_INPUT
{
	float3 pos : inl_Positions;
	float2 uv  : inl_TexCoords;
	float4 col : inl_Colour;
};

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
	float4 col : COLOR0;
};

PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output;
	output.pos = mul(projection_matrix, float4(input.pos.xy, 0.f, 1.f));
	output.col = input.col;
	output.uv = input.uv;
	return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
	return input.col * tex.Sample(sam, input.uv);
}