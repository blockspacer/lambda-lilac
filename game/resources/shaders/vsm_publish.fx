#include "resources/shaders/common.fx"

float linearStep(float min, float max, float val)
{
  return clamp((val - min) / (max - min), 0.0f, 1.0f);
}

#define OFFSET 0.1f

//#define NVIDIA_METHOD
//http://developer.download.nvidia.com/SDK/10.5/direct3d/Source/VarianceShadowMapping/Doc/VarianceShadowMapping.pdf

float calcShadow(float3 moments, float compare, float light_far)
{
#ifdef NVIDIA_METHOD
  float E_x2 = moments.y;
  float Ex_2 = moments.x * moments.x;
  float variance = E_x2 - Ex_2;
  float mD = moments.x - compare;
  float mD_2 = mD * mD;
  float p = variance / (variance + mD_2);

  return min(max(p, compare <= moments.x ), 1.0f);
#else
  float p = step(compare, moments.x);
  float variance = max(moments.y - moments.x * moments.x, 50.0f / light_far);

  float d = (compare - moments.x);
  float p_max = linearStep(0.2f, 1.0f, variance / (variance + d * d));

  return min(max(p, p_max), 1.0f);// * when_gt(moments.z, compare - OFFSET);
#endif
}