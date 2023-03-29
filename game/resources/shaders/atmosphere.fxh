#include "common.fxh"

#define iSteps 16
#define jSteps 8
#define iStepsMul iSteps * 3
#define jStepsMul jSteps

float2 rsi(float3 r0, float3 rd, float sr)
{
  // ray-sphere intersection that assumes
  // the sphere is centered at the origin.
  // No intersection when result.x > result.y
  float a = dot(rd, rd);
  float b = 2.0f * dot(rd, r0);
  float c = dot(r0, r0) - (sr * sr);
  float d = (b * b) - 4.0f * a * c;

  float lt = when_lt(d, 0.0f);
  return float2(
    (-b - sqrt(d)) / (2.0f * a),
    (-b + sqrt(d)) / (2.0f * a)
    ) * (1.0f - lt) + float2(1e5f, -1e5f) * lt;
}

/*
* param[in] r       (float3) normalized ray direction
* param[in] r0      (float3) ray origin
* param[in] pSun    (float3) position of the sun
* param[in] iSun    (float1) intensity of the sun
* param[in] rPlanet (float1) radius of the planet in meters
* param[in] rAtmos  (float1) radius of the atmosphere in meters
* param[in] kRlh    (float3) Rayleigh scattering coefficient
* param[in] kMie    (float1) Mie scattering coefficient
* param[in] shRlh   (float1) Rayleigh scale height
* param[in] shMie   (float1) Mie scale height
* param[in] g       (float1) Mie preferred scattering direction
*/
float3 atmosphere(float3 r, float3 r0, float3 pSun, float iSun, float rPlanet, float rAtmos, float3 kRlh, float kMie, float shRlh, float shMie, float g)
{
  // Normalize the sun and view directions.
  pSun = normalize(pSun);
  r = normalize(r);

  // Calculate the step size of the primary ray.
  float2 p = rsi(r0, r, rAtmos);
  if (p.x > p.y)
  {
    return float3(0.0f, 0.0f, 0.0f);
  }
  p.y = min(p.y, rsi(r0, r, rPlanet).x);
  float iStepSize = (p.y - p.x) / float(iStepsMul);

  // Initialize the primary ray time.
  float iTime = 0.0;

  // Initialize accumulators for Rayleigh and Mie scattering.
  float3 totalRlh = float3(0.0f, 0.0f, 0.0f);
  float3 totalMie = float3(0.0f, 0.0f, 0.0f);

  // Initialize optical depth accumulators for the primary ray.
  float iOdRlh = 0.0f;
  float iOdMie = 0.0f;

  // Calculate the Rayleigh and Mie phases.
  float mu = dot(r, pSun);
  float mumu = mu * mu;
  float gg = g * g;
  float pRlh = 3.0f / (16.0f * PI) *  (1.0f + mumu);
  float pMie = 3.0f / (8.0f  * PI) * ((1.0f - gg) * (mumu + 1.0f)) / (pow(abs(1.0f + gg - 2.0f * mu * g), 1.5f) * (2.0f + gg));

  // Sample the primary ray.
  for (int i = 0; i < iSteps; i++)
  {
    // Calculate the primary ray sample position.
    float3 iPos = r0 + r * (iTime + iStepSize * 0.5f);

    // Calculate the height of the sample.
    float iHeight = length(iPos) - rPlanet;

    // Calculate the optical depth of the Rayleigh and Mie scattering for this step.
    float odStepRlh = exp(-iHeight / shRlh) * iStepSize;
    float odStepMie = exp(-iHeight / shMie) * iStepSize;

    // Accumulate optical depth.
    iOdRlh += odStepRlh;
    iOdMie += odStepMie;

    // Calculate the step size of the secondary ray.
    float jStepSize = rsi(iPos, pSun, rAtmos).y / float(jStepsMul);

    // Initialize the secondary ray time.
    float jTime = 0.0f;

    // Initialize optical depth accumulators for the secondary ray.
    float jOdRlh = 0.0f;
    float jOdMie = 0.0f;

    // Sample the secondary ray.
    for (int j = 0; j < jSteps; j++)
    {

      // Calculate the secondary ray sample position.
      float3 jPos = iPos + pSun * (jTime + jStepSize * 0.5f);

      // Calculate the height of the sample.
      float jHeight = length(jPos) - rPlanet;

      // Accumulate the optical depth.
      jOdRlh += exp(-jHeight / shRlh) * jStepSize;
      jOdMie += exp(-jHeight / shMie) * jStepSize;

      // Increment the secondary ray time.
      jTime += jStepSize;
    }

    // Calculate attenuation.
    float3 attn = exp(-(kMie * (iOdMie + jOdMie) + kRlh * (iOdRlh + jOdRlh)));

    // Accumulate scattering.
    totalRlh += odStepRlh * attn;
    totalMie += odStepMie * attn;

    // Increment the primary ray time.
    iTime += iStepSize;
  }

  float3 rlh = pRlh * kRlh * totalRlh;
  float3 mie = pMie * kMie * totalMie;

  // Calculate and return the final color.
  return iSun * (rlh + mie);
}