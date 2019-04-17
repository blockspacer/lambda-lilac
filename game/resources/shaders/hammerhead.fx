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

Make_Texture2D(tex_hammerhead, 0);
static const uint SAMPLE_COUNT = 512u;

float RadicalInverse_VdC(uint bits)
{
  bits = (bits << 16u) | (bits >> 16u);
  bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
  bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
  bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
  bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
  return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

float2 Hammersley(uint i, uint N)
{
  return float2(float(i) / float(N), RadicalInverse_VdC(i));
}

float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness)
{
  float a = roughness * roughness;

  float phi = TAU * Xi.x;
  float cosTheta = sqrt((1.0f - Xi.y) / (1.0f + (a * a - 1.0f) * Xi.y));
  float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

  // from spherical coordinates to cartesian coordinates
  float3 H;
  H.x = cos(phi) * sinTheta;
  H.y = sin(phi) * sinTheta;
  H.z = cosTheta;

  // from tangent-space vector to world-space sample vector
  float3 up        = abs(N.z) < 0.999f ? float3(0.0f, 0.0f, 1.0f) : float3(1.0f, 0.0f, 0.0f);
  float3 tangent   = normalize(cross(up, N));
  float3 bitangent = cross(N, tangent);

  float3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
  return normalize(sampleVec);
} 

float4 PS(VSOutput pIn) : SV_TARGET0
{
  float3 N = normalize(SampleSphericalMap(pIn.tex));
  float3 R = N;
  float3 V = R;

  float totalWeight = 0.0;
  float3 prefilteredColor = 0.0f;
  for (uint i = 0u; i < SAMPLE_COUNT; ++i)
  {
    float2 Xi = Hammersley(i, SAMPLE_COUNT);
    float3 H  = ImportanceSampleGGX(Xi, N, user_data[0].x);
    float3 L  = normalize(2.0 * dot(V, H) * H - V);

    float NdotL = max(dot(N, L), 0.0);
    if (NdotL > 0.0)
    {
      prefilteredColor += tex_hammerhead.Sample(SamLinearClamp, SampleSphericalMap(L)).rgb * NdotL;
      totalWeight      += NdotL;
    }
  }
  prefilteredColor = prefilteredColor / totalWeight;

  return float4(prefilteredColor, 1.0f);
}
