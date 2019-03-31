#pragma once
#include <cstdint>

namespace lambda
{
  enum class ShaderDepthFunc : uint8_t
  {
    kNever = 0u,
    kLess = 1u,
    kEqual = 2u,
    kLessEqual = 3u,
    kGreater = 4u,
    kNotEqual = 5u,
    kGreaterEqual = 6u,
    kAlways = 7u
  };
  enum class ShaderBlendMode : uint8_t
  {
    kOverwrite = 0u,
    kAdd = 1u,
    kSubtract = 2u,
    kMultiply = 3u,
    kAlpha = 4u
  };
  enum class ShaderStages : uint8_t
  {
    kVertex,
    kGeometry,
    kPixel,
    kFragment = kPixel,
    kCompute,
    kDomain,
    kHull,
    kCount
  };
  enum class TextureFormat : uint8_t
  {
    kUnknown = 0u,
    kR8G8B8A8,
    kR16G16B16A16,
    kR32G32B32A32,
    kR16G16,
    kR32G32,
    kR32,
    kR16,
    kR24G8,
    kBC1,
    kBC2,
    kBC3,
    kBC4,
    kBC5,
    kBC6,
    kBC7,
    kB8G8R8A8,
		kA8,
		kD32,
  };
  // Bytes Per Pixel. Bytes Per Row. Bytes Per Layer.
  inline void calculateImageMemory(TextureFormat format, uint16_t w, uint16_t h, uint32_t& bpp, uint32_t& bpr, uint32_t& bpl)
  {
    switch (format)
    {
    default:
    case TextureFormat::kUnknown:
      bpp = bpr = bpl = 0u;
      break;
		case TextureFormat::kA8:
			bpp = 1;
			bpr = bpp * w;
			bpl = bpr * h;
			break;
    case TextureFormat::kR16:
      bpp = 2;
      bpr = bpp * w;
      bpl = bpr * h;
      break;
    case TextureFormat::kR8G8B8A8:
    case TextureFormat::kB8G8R8A8:
    case TextureFormat::kR16G16:
    case TextureFormat::kR32:
    case TextureFormat::kR24G8:
      bpp = 4;
      bpr = bpp * w;
      bpl = bpr * h;
      break;
    case TextureFormat::kR16G16B16A16:
    case TextureFormat::kR32G32:
      bpp = 8;
      bpr = bpp * w;
      bpl = bpr * h;
      break; 
    case TextureFormat::kR32G32B32A32:
      bpp = 16;
      bpr = bpp * w;
      bpl = bpr * h;
      break;
    case TextureFormat::kBC1:
    case TextureFormat::kBC5:
      bpp = 1; // BPP is not really usable in this case.
      bpr = (w + 3u) / 4u * 8u;
      if (bpr < 8u)
        bpr = 8u;
      bpl = w * h / 2u;
      if (bpl < 8u)
        bpl = 8u;
      break;
    case TextureFormat::kBC2:
    case TextureFormat::kBC3:
    case TextureFormat::kBC4:
    case TextureFormat::kBC6:
    case TextureFormat::kBC7:
      bpp = 1; // BPP is not really usable in this case.
      bpr = (w + 3u) / 4u * 16u;
      if (bpr < 16u)
        bpr = 16u;
      bpl = w * h;
      if (bpl < 16u)
        bpl = 16u;
      break;
    }
  }
}