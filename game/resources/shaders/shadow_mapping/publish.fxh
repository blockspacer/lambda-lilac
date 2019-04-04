#ifndef __SHADOW_MAPPING_PUBLISH__
#define __SHADOW_MAPPING_PUBLISH__

#if VSM
float linearStep(float min, float max, float val)
{
  return clamp((val - min) / (max - min), 0.0f, 1.0f);
}

float calcShadow(float2 moments, float compare)
{
  float p = step(compare, moments.x);
  float variance = max(moments.y - moments.x * moments.x, 0.001f);

  float d = (compare - moments.x);
  float p_max = linearStep(0.2f, 1.0f, variance / (variance + d * d));

  return min(max(p, p_max), 1.0f);
}
#elif ESM
static const float kMultiplier = 2.0f;

float calcShadow(float2 moments, float compare)
{
  return saturate(clamp(exp((moments.x - compare) * kMultiplier), 0.0f, 1.0f));
}
#else
float calcShadow(float2 moments, float compare)
{
  return 1.0f;
}
#endif
#endif // __SHADOW_MAPPING_PUBLISH__