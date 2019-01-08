#include <FastNoise.h>
#include "script_vector.h"

namespace lambda
{
  namespace scripting
  {
    ///////////////////////////////////////////////////////////////////////////
    class ScriptNoise
    {
    public:
      enum Interpolation
      {
        kLinear,
        kHermite,
        kQuintic
      };

    public:
      void SetSeed(const int& seed);
      int GetSeed() const;
      void SetFrequency(const float& frequency);
      float GetFrequency() const;
      void SetInterpolation(const int& Interpolation);
      int GetInterpolation() const;

      float GetPerlin        (const ScriptVec2& id) const;
      float GetPerlin        (const ScriptVec3& id) const;
      float GetPerlinFractal (const ScriptVec2& id) const;
      float GetPerlinFractal (const ScriptVec3& id) const;
      float GetCellular      (const ScriptVec2& id) const;
      float GetCellular      (const ScriptVec3& id) const;
      float GetCubic         (const ScriptVec2& id) const;
      float GetCubic         (const ScriptVec3& id) const;
      float GetCubicFractal  (const ScriptVec2& id) const;
      float GetCubicFractal  (const ScriptVec3& id) const;
      float GetSimplex       (const ScriptVec2& id) const;
      float GetSimplex       (const ScriptVec3& id) const;
      float GetSimplexFractal(const ScriptVec2& id) const;
      float GetSimplexFractal(const ScriptVec3& id) const;
      float GetValue         (const ScriptVec2& id) const;
      float GetValue         (const ScriptVec3& id) const;
      float GetValueFractal  (const ScriptVec2& id) const;
      float GetValueFractal  (const ScriptVec3& id) const;
      float GetWhiteNoise    (const ScriptVec2& id) const;
      float GetWhiteNoise    (const ScriptVec3& id) const;

    private:
      FastNoise fast_noise_;
    };
  }
}