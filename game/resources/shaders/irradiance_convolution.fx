#include "common.fxh"

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

Make_Texture2D(tex_environment, 0);

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
  for(float phi = 0.0f; phi < TAU; phi += sampleDelta)
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
