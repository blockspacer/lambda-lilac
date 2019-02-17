#include "common.fx"
#include "atmosphere.fx"

cbuffer cbPerPass
{
  float4x4 model_matrix;
  float4x4 view_matrix;
  float4x4 projection_matrix;
  float4x4 inverse_projection_matrix;
};

cbuffer cbPerFrameSky
{
  float3 eSunDir = normalize(float3(1.0f, 1.0f, 1.0f));
  float eNearPlane = 0.1f;
  float eFarPlane = 1000.0f;
};

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

Texture2D   tex_post_process : register(t0);
Texture2D   tex_position     : register(t1);
#ifdef CUBEMAP
TextureCube tex_cubemap      : register(t2);
#else
Texture2D   tex_cubemap      : register(t2);
#endif

#define FLT_MAX 3.402823466e+5F
#define NEW_METHOD
struct PSOutput
{
  float4 albedo   : SV_TARGET0;
  float4 position : SV_TARGET1;
};

PSOutput PS(VSOutput pIn)
{
  float3 input = float3(
    (pIn.tex.x * 2.0f - 1.0f),
    -(pIn.tex.y * 2.0f - 1.0f),
    1.0f
  );

  float3 normal       = normalize(mul(mul(inverse_projection_matrix, float4(input, 1.0f)).xyz, (float3x3)view_matrix));
  float4 post_process = Sample(tex_post_process, SamLinearClamp, pIn.tex);
  float4 position     = Sample(tex_position, SamLinearClamp, pIn.tex);

#ifdef NEW_METHOD
#ifdef CUBEMAP
  float4 skybox = pow(tex_cubemap.Sample(SamLinearClamp, normalize(normal)), 2.2f);
#else
  float4 skybox = tex_cubemap.Sample(SamLinearClamp, SampleSphericalMap(normal));
#endif
#else
  float4 skybox = float4(atmosphere(
    normal,                              // normalized ray direction
    float3(0.0f, 6372e3f, 0.0f),         // ray origin
    eSunDir,                             // position of the sun
    22.0f,                               // intensity of the sun
    6371e3f,                             // radius of the planet in meters
    6471e3f,                             // radius of the atmosphere in meters
    float3(5.5e-6f, 13.0e-6f, 22.4e-6f), // Rayleigh scattering coefficient
    21e-7f,                              // Mie scattering coefficient
    8e3f,                                // Rayleigh scale height
    1.2e4f,                              // Mie scale height
    0.758f                               // Mie preferred scattering direction
  ), 1.0f);
  
  // Apply exposure.
  skybox = 1.0f - exp(-1.0f * skybox);
#endif

  PSOutput pOut;
  pOut.albedo   = lerp(skybox, post_process, post_process.a);
  pOut.position = lerp(FLT_MAX, position, post_process.a);
  return pOut;
}