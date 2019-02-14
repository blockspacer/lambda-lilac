#define PD_MEDIUM
#include "poisson_disc.fx"
#include "common.fx"

static const float rsm_intensity = 1.0f;
static const float rsm_r_max = 0.05f;

Texture2D tex_s_position   : register(t5);
Texture2D tex_s_normal     : register(t6);
Texture2D tex_s_flux       : register(t7);

float3 RSM(float2 shadow_coord, float3 w_normal, float3 w_position)
{
  float3 indirect_illumination = float3(0.0f, 0.0f, 0.0f);
 
  for (uint i = 0; i < POISSON_DISC_SAMPLE_COUNT; ++i)
  {
    float2 rnd = poisson_disc[i];

    float2 coords = shadow_coord + rsm_r_max * rnd;

    float3 s_position = tex_s_position.Sample(SamLinearBorder, coords).xyz;
    float3 s_normal   = normalize(tex_s_normal.Sample(SamLinearBorder, coords).xyz * 2.0f - 1.0f);
    float3 flux       = tex_s_flux.Sample(SamLinearBorder, coords).xyz;
    
    float3 result = (flux * 
      ((max(0.0f, dot(s_normal, w_position - s_position)) * 
      max(0.0f, dot(w_normal, s_position - w_position))) / 
      pow(length(w_position - s_position), 4.0f))
    );

    result *= rnd.x * rnd.x;
    indirect_illumination += result;
  }
  return max(0.0f, saturate(indirect_illumination * rsm_intensity));
}
