#include "resources/shaders/common.fx"

struct VSOutput
{
  float4 position : SV_POSITION0;
  float2 tex      : TEX_COORD;
};

static const float2 invAtan = float2(0.1591, 0.3183);
float2 SampleSphericalMap(float3 v)
{
  float2 uv = float2(atan2(v.z, v.x), -asin(v.y));
  return uv * invAtan + 0.5f;
}
float3 SampleSphericalMap(float2 uv)
{
  float2 thetaphi = ((uv * 2.0f) - 1.0f) * float2(3.1415926535897932384626433832795f, 1.5707963267948966192313216916398f);
  return float3(cos(thetaphi.y) * cos(thetaphi.x), -sin(thetaphi.y), cos(thetaphi.y) * sin(thetaphi.x));
}

VSOutput VS(uint id: SV_VertexID)
{
  VSOutput vOut;
  vOut.tex = float2((id << 1) & 2, id & 2);
  vOut.position = float4(vOut.tex * float2(2, -2) + float2(-1, 1), 0, 1);
  return vOut;
}

Texture2D tex_environment : register(t0);
static const float PI = 3.14159265359f;

float4 PS(VSOutput pIn) : SV_TARGET0
{
  float3 N = normalize(SampleSphericalMap(pIn.tex));
  float3 irradiance = 0.0f;   
  
  // tangent space calculation from origin point
  float3 up    = float3(0.0f, 1.0f, 0.0f);
  float3 right = cross(up, N);
  up           = cross(N, right);

  float sampleDelta = 0.025f;
  float nrSamples   = 0.0f;
  for(float phi = 0.0f; phi < 2.0f * PI; phi += sampleDelta)
  {
    for(float theta = 0.0f; theta < 0.5f * PI; theta += sampleDelta)
    {
      // spherical to cartesian (in tangent space)
      float3 tangentSample = float3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
      // tangent space to world
      float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 

      irradiance += tex_environment.Sample(SamLinearClamp, SampleSphericalMap(sampleVec)).rgb * cos(theta) * cos(theta);
      nrSamples++;
    }
  }
  irradiance = PI * irradiance * (1.0 / float(nrSamples));
  
  return float4(irradiance, 1.0f);
}
