#include "common.fx"

Make_CBuffer(cbPostProcess, 0)
{
  float2 screen_size;

  float iFxaaSpanMax = 8.0f;
  float iFxaaReduceMin = 1.0f / 128.0f;
  float iFxaaReduceMul = 1.0f / 8.0f;
};

struct VSOutput
{
  float4 position : SV_POSITION;
  float2 tex      : TEXCOORD;
};

VSOutput VS(uint id: SV_VertexID)
{
  VSOutput vOut;
  vOut.tex = float2((id << 1) & 2, id & 2);
  vOut.position = float4(vOut.tex * float2(2, -2) + float2(-1, 1), 0, 1);
  return vOut;
}

Make_Texture2D(tex_albedo, 0);

float4 PS(VSOutput pIn) : SV_TARGET
{
  float2 texel_size = 1.0f / screen_size;
  float3 luma = float3(0.299f, 0.587f, 0.114f);

  float lumaTL = dot(luma, Sample(tex_albedo, SamLinearClamp, pIn.tex + (float2(-1.0f, -1.0f) * texel_size)).rgb);
  float lumaTR = dot(luma, Sample(tex_albedo, SamLinearClamp, pIn.tex + (float2(+1.0f, -1.0f) * texel_size)).rgb);
  float lumaBL = dot(luma, Sample(tex_albedo, SamLinearClamp, pIn.tex + (float2(-1.0f, +1.0f) * texel_size)).rgb);
  float lumaBR = dot(luma, Sample(tex_albedo, SamLinearClamp, pIn.tex + (float2(+1.0f, +1.0f) * texel_size)).rgb);
  float lumaMM = dot(luma, Sample(tex_albedo, SamLinearClamp, pIn.tex).rgb);

  float2 blur_dir = float2(
    -((lumaTL + lumaTR) - (lumaBL + lumaBR)), // Possible error.
    ((lumaTL + lumaBL) - (lumaTR + lumaBR))
  );

  const float magnitude_reduction = max(
    (lumaTL + lumaTR + lumaBL + lumaBR) * (0.25f * iFxaaReduceMul),
    iFxaaReduceMin
  );
  float min_magnitude = 1.0f / (min(abs(blur_dir.x), abs(blur_dir.y)) + magnitude_reduction);

  blur_dir = min(
    float2(iFxaaSpanMax, iFxaaSpanMax),
    max(
      float2(-iFxaaSpanMax, -iFxaaSpanMax),
      blur_dir * min_magnitude
    )
  ) * texel_size;

  float3 result1 = 0.5f * (
    Sample(tex_albedo, SamLinearClamp, pIn.tex + blur_dir * (1.0f / 3.0f - 0.5f)).rgb +
    Sample(tex_albedo, SamLinearClamp, pIn.tex + blur_dir * (2.0f / 3.0f - 0.5f)).rgb
    );

  float3 result2 = result1 * 0.5f + 0.25f * (
    Sample(tex_albedo, SamLinearClamp, pIn.tex + blur_dir * (0.0f / 3.0f - 0.5f)).rgb +
    Sample(tex_albedo, SamLinearClamp, pIn.tex + blur_dir * (3.0f / 3.0f - 0.5f)).rgb
    );

  float luma_min = min(lumaMM, min(lumaTL, min(lumaTR, min(lumaBL, lumaBR))));
  float luma_max = max(lumaMM, max(lumaTL, max(lumaTR, max(lumaBL, lumaBR))));
  float luma_result_2 = dot(luma, result2);

  if (luma_result_2 < luma_min || luma_result_2 > luma_max)
  {
    return float4(result1, Sample(tex_albedo, SamLinearClamp, pIn.tex).a);
  }
  else
  {
    return float4(result2, Sample(tex_albedo, SamLinearClamp, pIn.tex).a);
  }
}
