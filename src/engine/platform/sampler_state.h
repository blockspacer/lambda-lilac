#pragma once

namespace lambda
{
  namespace platform
  {
    ///////////////////////////////////////////////////////////////////////////
    class SamplerState
    {
    public:
      /////////////////////////////////////////////////////////////////////////
      enum class SampleMode : unsigned char
      {
        kPoint,
        kLinear,
        kAnisotropic,
      };

      /////////////////////////////////////////////////////////////////////////
      enum class ClampMode : unsigned char
      {
        kClamp,
        kBorder,
        kWrap,
      }; 

      SamplerState() 
        : sample_mode_(SampleMode::kPoint)
        , clamp_mode_(ClampMode::kClamp) {}
      SamplerState(SampleMode sample_mode, ClampMode clamp_mode) 
        : sample_mode_(sample_mode)
        , clamp_mode_(clamp_mode) {}
      SamplerState(SampleMode sample_mode) 
        : sample_mode_(sample_mode)
        , clamp_mode_(ClampMode::kClamp) {}
      SamplerState(ClampMode clamp_mode) 
        : sample_mode_(SampleMode::kPoint)
        , clamp_mode_(clamp_mode) {}

      bool operator==(const SamplerState& other) const
      {
        return sample_mode_ == other.sample_mode_ && 
          clamp_mode_ == other.clamp_mode_;
      }

      static SamplerState PointWrap()
      {
        return SamplerState(
          SampleMode::kPoint,
          ClampMode::kWrap
        );
      }
      static SamplerState LinearWrap()
      {
        return SamplerState(
          SampleMode::kLinear,
          ClampMode::kWrap
        );
      }

      static SamplerState PointClamp()
      {
        return SamplerState(
          SampleMode::kPoint,
          ClampMode::kClamp
        );
      }
      static SamplerState LinearClamp()
      {
        return SamplerState(
          SampleMode::kLinear,
          ClampMode::kClamp
        );
      }

      static SamplerState PointBorder()
      {
        return SamplerState(
          SampleMode::kPoint,
          ClampMode::kBorder
        );
      }
      static SamplerState LinearBorder()
      {
        return SamplerState(
          SampleMode::kLinear,
          ClampMode::kBorder
        );
      }
      
      inline SampleMode getSampleMode() const { return sample_mode_; }
      inline ClampMode getClampMode() const { return clamp_mode_; }

    private:
      SampleMode sample_mode_;
      ClampMode clamp_mode_;
    };
  }
}