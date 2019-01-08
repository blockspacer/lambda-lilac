#include "script_noise.h"

namespace lambda
{
  namespace scripting
  {
    ///////////////////////////////////////////////////////////////////////////
    void ScriptNoise::SetSeed(const int& seed)
    {
      fast_noise_.SetSeed(seed);
    }
  
    ///////////////////////////////////////////////////////////////////////////
    int ScriptNoise::GetSeed() const
    {
      return fast_noise_.GetSeed();
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void ScriptNoise::SetFrequency(const float& frequency)
    {
      fast_noise_.SetFrequency(frequency);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    float ScriptNoise::GetFrequency() const
    {
      return fast_noise_.GetFrequency();
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void ScriptNoise::SetInterpolation(const int& interpolation)
    {
      switch (interpolation)
      {
      case Interpolation::kLinear:
        fast_noise_.SetInterp(FastNoise::Interp::Linear);
        break;
      case Interpolation::kHermite:
        fast_noise_.SetInterp(FastNoise::Interp::Hermite);
        break;
      default:
      case Interpolation::kQuintic:
        fast_noise_.SetInterp(FastNoise::Interp::Quintic);
        break;
      }
    }
    
    ///////////////////////////////////////////////////////////////////////////
    int ScriptNoise::GetInterpolation() const
    {
      FastNoise::Interp i = fast_noise_.GetInterp();
      switch (i)
      {
      case FastNoise::Interp::Linear:
        return Interpolation::kLinear;
        break;
      case FastNoise::Interp::Hermite:
        return Interpolation::kHermite;
        break;
      default:
      case FastNoise::Interp::Quintic:
        return Interpolation::kQuintic;
        break;
      }
    }
    
    ///////////////////////////////////////////////////////////////////////////
    float ScriptNoise::GetPerlin(const ScriptVec2& id) const
    {
      return fast_noise_.GetPerlin(id.x, id.y);
    }
  
    ///////////////////////////////////////////////////////////////////////////
    float ScriptNoise::GetPerlin(const ScriptVec3& id) const
    {
      return fast_noise_.GetPerlin(id.x, id.y, id.z);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    float ScriptNoise::GetPerlinFractal(const ScriptVec2& id) const
    {
      return fast_noise_.GetPerlinFractal(id.x, id.y);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    float ScriptNoise::GetPerlinFractal(const ScriptVec3& id) const
    {
      return fast_noise_.GetPerlinFractal(id.x, id.y, id.z);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    float ScriptNoise::GetCellular(const ScriptVec2& id) const
    {
      return fast_noise_.GetCellular(id.x, id.y);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    float ScriptNoise::GetCellular(const ScriptVec3& id) const
    {
      return fast_noise_.GetCellular(id.x, id.y, id.z);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    float ScriptNoise::GetCubic(const ScriptVec2& id) const
    {
      return fast_noise_.GetCubic(id.x, id.y);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    float ScriptNoise::GetCubic(const ScriptVec3& id) const
    {
      return fast_noise_.GetCubic(id.x, id.y, id.z);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    float ScriptNoise::GetCubicFractal(const ScriptVec2& id) const
    {
      return fast_noise_.GetCubicFractal(id.x, id.y);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    float ScriptNoise::GetCubicFractal(const ScriptVec3& id) const
    {
      return fast_noise_.GetCubicFractal(id.x, id.y, id.z);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    float ScriptNoise::GetSimplex(const ScriptVec2& id) const
    {
      return fast_noise_.GetSimplex(id.x, id.y);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    float ScriptNoise::GetSimplex(const ScriptVec3& id) const
    {
      return fast_noise_.GetSimplex(id.x, id.y, id.z);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    float ScriptNoise::GetSimplexFractal(const ScriptVec2& id) const
    {
      return fast_noise_.GetSimplexFractal(id.x, id.y);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    float ScriptNoise::GetSimplexFractal(const ScriptVec3& id) const
    {
      return fast_noise_.GetSimplexFractal(id.x, id.y, id.z);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    float ScriptNoise::GetValue(const ScriptVec2& id) const
    {
      return fast_noise_.GetValue(id.x, id.y);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    float ScriptNoise::GetValue(const ScriptVec3& id) const
    {
      return fast_noise_.GetValue(id.x, id.y, id.z);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    float ScriptNoise::GetValueFractal(const ScriptVec2& id) const
    {
      return fast_noise_.GetValueFractal(id.x, id.y);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    float ScriptNoise::GetValueFractal(const ScriptVec3& id) const
    {
      return fast_noise_.GetValueFractal(id.x, id.y, id.z);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    float ScriptNoise::GetWhiteNoise(const ScriptVec2& id) const
    {
      return fast_noise_.GetWhiteNoise(id.x, id.y);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    float ScriptNoise::GetWhiteNoise(const ScriptVec3& id) const
    {
      return fast_noise_.GetWhiteNoise(id.x, id.y, id.z);
    }
  }
}