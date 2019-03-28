#pragma once

namespace lambda
{
  namespace platform
  {
    ///////////////////////////////////////////////////////////////////////////
    class BlendState
    {
    public:
      /////////////////////////////////////////////////////////////////////////
      enum class BlendMode : unsigned char
      {
        kZero,
        kOne,
        kSrcColour,
        kInvSrcColour,
        kSrcAlpha,
        kInvSrcAlpha,
        kDestAlpha,
        kInvDestAlpha,
        kDestColour,
        kInvDestColour,
        kSrcAlphaSat,
        kBlendFactor,
        kInvBlendFactor,
        kSrc1Colour,
        kInvSrc1Colour,
        kSrc1Alpha,
        kInvSrc1Alpha,
      };

      /////////////////////////////////////////////////////////////////////////
      enum class BlendOp : unsigned char
      {
        kAdd,
        kSubtract,
        kRevSubtract,
        kMin,
        kMax,
      };
     
      /////////////////////////////////////////////////////////////////////////
      enum class WriteMode : unsigned char
      {
        kColourWriteEnableRed   = 1,
        kColourWriteEnableGreen = 2,
        kColourWriteEnableBlue  = 4,
        kColourWriteEnableAlpha = 8,
        kColourWriteEnableRGB   = 
          ((kColourWriteEnableRed | kColourWriteEnableGreen) | 
          kColourWriteEnableBlue),
        kColourWriteEnableRGBA  = 
          (kColourWriteEnableRGB | kColourWriteEnableAlpha)
      };

      BlendState() :
        alpha_to_coverage_(false),
        blend_enable_(false),
        src_blend_(BlendMode::kOne),
        dest_blend_(BlendMode::kOne),
        blend_op_(BlendOp::kAdd),
        src_blend_alpha_(BlendMode::kOne),
        dest_blend_alpha_(BlendMode::kOne),
        blend_op_alpha_(BlendOp::kAdd),
        write_mask_((unsigned char)WriteMode::kColourWriteEnableRGBA)
      {}

      BlendState(
        bool alpha_to_coverage,
        bool blend_enable,
        BlendMode src_blend,
        BlendMode dest_blend,
        BlendOp blend_op,
        BlendMode src_blend_alpha,
        BlendMode dest_blend_alpha,
        BlendOp blend_op_alpha,
        unsigned char write_mask
      ) :
        alpha_to_coverage_(alpha_to_coverage),
        blend_enable_(blend_enable),
        src_blend_(src_blend),
        dest_blend_(dest_blend),
        blend_op_(blend_op),
        src_blend_alpha_(src_blend_alpha),
        dest_blend_alpha_(dest_blend_alpha),
        blend_op_alpha_(blend_op_alpha),
        write_mask_(write_mask)
      {}

      bool operator==(const BlendState& other) const
      {
        return 
          alpha_to_coverage_ == other.alpha_to_coverage_&&
          blend_enable_ == other.blend_enable_&&
          src_blend_ == other.src_blend_&&
          dest_blend_ == other.dest_blend_&&
          blend_op_ == other.blend_op_&&
          src_blend_alpha_ == other.src_blend_alpha_&&
          dest_blend_alpha_ == other.dest_blend_alpha_&&
          write_mask_ == other.write_mask_
          ;
      }

      static BlendState Alpha(const bool& alpha_to_coverage = false)
      {
        return BlendState(
          alpha_to_coverage,
          true,
          BlendMode::kSrcAlpha,
          BlendMode::kInvSrcAlpha,
          BlendOp::kAdd,
          BlendMode::kOne,
          BlendMode::kInvSrcAlpha,
          BlendOp::kAdd,
          (unsigned char)WriteMode::kColourWriteEnableRGBA
        );
      }

      static BlendState Default()
      {
        return BlendState{
          false,
          false,
          BlendMode::kSrcAlpha,
          BlendMode::kInvSrcAlpha,
          BlendOp::kAdd,
          BlendMode::kOne,
          BlendMode::kInvSrcAlpha,
          BlendOp::kAdd,
          (unsigned char)WriteMode::kColourWriteEnableRGBA
        };
      }

      static BlendState Additive()
      {
        return BlendState{
          false,
          true,
          BlendMode::kOne,
          BlendMode::kOne,
          BlendOp::kAdd,
          BlendMode::kOne,
          BlendMode::kOne,
          BlendOp::kAdd,
          (unsigned char)WriteMode::kColourWriteEnableRGBA
        };
      }

      inline bool getAlphaToCoverage() const { return alpha_to_coverage_; }
      inline bool getBlendEnable() const { return blend_enable_; }
      inline BlendMode getSrcBlend() const { return src_blend_; }
      inline BlendMode getDestBlend() const { return dest_blend_; }
      inline BlendOp getBlendOp() const { return blend_op_; }
      inline BlendMode getSrcBlendAlpha() const { return src_blend_alpha_; }
      inline BlendMode getDestBlendAlpha() const { return dest_blend_alpha_; }
      inline BlendOp getBlendOpAlpha() const { return blend_op_alpha_; }
      inline unsigned char getWriteMask() const { return write_mask_; }

    private:
      bool alpha_to_coverage_;
      bool blend_enable_;
      BlendMode src_blend_;
      BlendMode dest_blend_;
      BlendOp blend_op_;
      BlendMode src_blend_alpha_;
      BlendMode dest_blend_alpha_;
      BlendOp blend_op_alpha_;
      unsigned char write_mask_;
    };
  }
}

namespace eastl 
{
	/////////////////////////////////////////////////////////////////////////////
	template <>
	struct hash<lambda::platform::BlendState>
	{
		std::size_t operator()(const lambda::platform::BlendState& k) const
		{
			size_t hash = 0ull;
			lambda::hashCombine(hash, (size_t)(k.getBlendEnable() ? 1 : 0));
			lambda::hashCombine(hash, (size_t)k.getBlendOp());
			lambda::hashCombine(hash, (size_t)k.getBlendOpAlpha());
			lambda::hashCombine(hash, (size_t)k.getDestBlend());
			lambda::hashCombine(hash, (size_t)k.getDestBlendAlpha());
			lambda::hashCombine(hash, (size_t)k.getSrcBlend());
			lambda::hashCombine(hash, (size_t)k.getSrcBlendAlpha());
			lambda::hashCombine(hash, (size_t)k.getWriteMask());
			return hash;
		}
	};
}
