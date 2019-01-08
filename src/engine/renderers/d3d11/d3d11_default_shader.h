#pragma once

///////////////////////////////////////////////////////////////////////////////
char kDefaultShader[] = R"(
struct VSInput
{
  float3 position : POSITION;
  float3 normal   : NORMAL;
  float2 tex      : TEX_COORD;
  float3 tangent  : TANGENT;
};

struct VSOutput
{
  float4 position  : SV_POSITION0;
  float4 hPosition : H_POSITION;
  float3 normal    : NORMAL;
  float4 colour    : COLOUR;
  float2 tex       : TEX_COORD;
  float3x3 tbn     : TBN;
};

cbuffer cbPerMesh
{
  float4x4 model_matrix;
  float4x4 view_matrix;
  float4x4 projection_matrix;
}

VSOutput VS(VSInput vIn)
{
  VSOutput vOut;
  vOut.hPosition = mul(model_matrix, float4(vIn.position, 1.0f));
  vOut.position  = mul(mul(projection_matrix, view_matrix), vOut.hPosition);
  vOut.colour    = float4(1.0f, 1.0f, 1.0f, 1.0f);
  vOut.tex       = vIn.tex;
  vOut.normal    = normalize(mul((float3x3)model_matrix, vIn.normal));

  float3 t = mul((float3x3)model_matrix), vIn.tangent));
  float3 n = normalize(mul((float3x3)model_matrix, vIn.normal));
  float3 b = mul((float3x3)model_matrix), cross(n, t)));
  vOut.tbn = transpose(float3x3(t, b, n));

  return vOut;
}

Texture2D tex_albedo   : register(t0);
Texture2D tex_normal   : register(t1);
Texture2D tex_mr       : register(t2);
SamplerState sam_point : register(s0);

struct PSOutput
{
  float4 albedo   : SV_Target0;
  float4 position : SV_Target1; 
  float4 normal   : SV_Target2; 
  float4 mr       : SV_Target3; // metallic roughness.
};

cbuffer cbPerMesh
{
  float2 metallic_roughness;
  float3 camera_position;
}

//TBN by Christian Schuler from http://www.thetenthplanet.de/archives/1180
float3x3 cotangent_frame(float3 N, float3 p, float2 uv)
{
  // get edge vectors of the pixel triangle
  float3 dp1 = ddx(p);
  float3 dp2 = ddy(p);
  float2 duv1 = ddx(uv);
  float2 duv2 = ddy(uv);

  // solve the linear system
  float3 dp2perp = cross(dp2, N);
  float3 dp1perp = cross(N, dp1);
  float3 T = dp2perp * duv1.x + dp1perp * duv2.x;
  float3 B = dp2perp * duv1.y + dp1perp * duv2.y;

  // construct a scale-invariant frame 
  float invmax = rsqrt(max(dot(T, T), dot(B, B)));
  return float3x3(T * invmax, B * invmax, N);
}

float3 perturb_normal(float3 N, float3 V, float2 texcoord)
{
  // assume N, the interpolated vertex normal and 
  // V, the view vector (vertex to eye)
  float3 map = (tex_normal.Sample(sam_point, texcoord).xyz) * 2.0f - 1.0f;
  float3x3 TBN = cotangent_frame(N, -V, texcoord);
  return normalize(mul(transpose(TBN), map));
}

PSOutput PS(VSOutput pIn)
{
  float3 N = normalize(pIn.normal);    
  float3 V = normalize(pIn.hPosition.xyz - camera_position);    
  float3 normal = perturb_normal(N, V, pIn.tex);

  
  normal = (tex_normal.Sample(sam_point, pIn.tex).xyz) * 2.0f - 1.0f;
  normal = normalize(mul(pIn.tbn, normal));
  normal = pIn.normal;

  float2 mr    = tex_mr.Sample(sam_point, pIn.tex).gb/* * metallic_roughness*/;
  
  PSOutput pOut;
  pOut.albedo   = tex_albedo.Sample(sam_point, pIn.tex) * pIn.colour;
  pOut.position = float4(pIn.hPosition.xyz, pOut.albedo.a);
  pOut.normal   = float4(normal * 0.5f + 0.5f, pOut.albedo.a);
  pOut.mr       = float4(mr, 1.0f, pOut.albedo.a);
  return pOut;
})";
