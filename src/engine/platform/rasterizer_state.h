#pragma once

namespace lambda
{
  namespace platform
  {
    ///////////////////////////////////////////////////////////////////////////
    class RasterizerState
    {
    public:
      /////////////////////////////////////////////////////////////////////////
      enum class FillMode : unsigned char
      {
        kWireframe,
        kSolid,
      };

      /////////////////////////////////////////////////////////////////////////
      enum class CullMode : unsigned char
      {
        kNone,
        kFront,
        kBack,
      };

      RasterizerState() 
        : fill_mode_(FillMode::kSolid)
        , cull_mode_(CullMode::kFront) {}
      RasterizerState(FillMode fill_mode, CullMode cull_mode) 
        : fill_mode_(fill_mode)
        , cull_mode_(cull_mode) {}
      RasterizerState(CullMode cull_mode) 
        : fill_mode_(FillMode::kSolid)
        , cull_mode_(cull_mode) {}
      RasterizerState(FillMode fill_mode) 
        : fill_mode_(fill_mode)
        , cull_mode_(CullMode::kFront) {}
      
      static RasterizerState SolidFront()
      {
        return RasterizerState(FillMode::kSolid, CullMode::kFront);
      }
      static RasterizerState SolidBack()
      {
        return RasterizerState(FillMode::kSolid, CullMode::kBack);
      }
      static RasterizerState SolidNone()
      {
        return RasterizerState(FillMode::kSolid, CullMode::kNone);
      }
      static RasterizerState WireframeFront()
      {
        return RasterizerState(FillMode::kWireframe, CullMode::kFront);
      }
      static RasterizerState WireframeBack()
      {
        return RasterizerState(FillMode::kWireframe, CullMode::kBack);
      }
      static RasterizerState WireframeNone()
      {
        return RasterizerState(FillMode::kWireframe, CullMode::kNone);
      }

      inline FillMode getFillMode() const { return fill_mode_; }
      inline CullMode getCullMode() const { return cull_mode_; }

      bool operator==(const RasterizerState& other) const
      {
        return fill_mode_ == other.fill_mode_&& cull_mode_ == other.cull_mode_;
      }

    private:
      FillMode fill_mode_;
      CullMode cull_mode_;
    };
  }
}