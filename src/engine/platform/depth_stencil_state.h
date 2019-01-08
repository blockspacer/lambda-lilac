#pragma once
#include <cstdint>

namespace lambda
{
  namespace platform
  {
    ///////////////////////////////////////////////////////////////////////////
    class DepthStencilState
    {
    public:
      enum class DepthCompareOp : unsigned char
      {
        kNever,
        kAlways,
        kEqual,
        kNotEqual,
        kLess,
        kLessEqual,
        kGreater,
        kGreaterEqual,
      };

      DepthStencilState() 
        : depth_compare_op_(DepthCompareOp::kLess)
        , depth_write_mode_(0xff) {}
      DepthStencilState(DepthCompareOp depth_compare_op) 
        : depth_compare_op_(depth_compare_op)
        , depth_write_mode_(0xff) {}
      DepthStencilState(
        DepthCompareOp depth_compare_op, 
        uint32_t depth_write_mode) 
        : depth_compare_op_(depth_compare_op)
        , depth_write_mode_(depth_write_mode) {}
      
      bool operator==(const DepthStencilState& other) const
      {
        return depth_compare_op_ == other.depth_compare_op_;
      }
      
      inline DepthCompareOp getDepthCompareOp() const { return depth_compare_op_; }
      inline uint32_t getDepthWritemode() const { return depth_write_mode_; }

      static DepthStencilState Default()
      {
        return DepthStencilState();
      }

      static DepthStencilState Never()
      {
        return DepthStencilState(DepthCompareOp::kNever);
      }
      static DepthStencilState Always()
      {
        return DepthStencilState(DepthCompareOp::kAlways);
      }
      static DepthStencilState Equal()
      {
        return DepthStencilState(DepthCompareOp::kEqual);
      }
      static DepthStencilState NotEqual()
      {
        return DepthStencilState(DepthCompareOp::kNotEqual);
      }
      static DepthStencilState Less()
      {
        return DepthStencilState(DepthCompareOp::kLess);
      }
      static DepthStencilState LessEqual()
      {
        return DepthStencilState(DepthCompareOp::kLessEqual);
      }
      static DepthStencilState Greater()
      {
        return DepthStencilState(DepthCompareOp::kGreater);
      }
      static DepthStencilState GreaterEqual()
      {
        return DepthStencilState(DepthCompareOp::kGreaterEqual);
      }

    private:
      DepthCompareOp depth_compare_op_;
      uint32_t depth_write_mode_;
    };
  }
}