#pragma once
#include "input.h"
#include <glm/glm.hpp>
#include <containers/containers.h>

namespace lambda
{
  namespace io
  {
    ///////////////////////////////////////////////////////////////////////////
    enum class KeyboardKeys : uint16_t
    {
      kA = 65,
      kB = 66,
      kC = 67,
      kD = 68,
      kE = 69,
      kF = 70,
      kG = 71,
      kH = 72,
      kI = 73,
      kJ = 74,
      kK = 75,
      kL = 76,
      kM = 77,
      kN = 78,
      kO = 79,
      kP = 80,
      kQ = 81,
      kR = 82,
      kS = 83,
      kT = 84,
      kU = 85,
      kV = 86,
      kW = 87,
      kX = 88,
      kY = 89,
      kZ = 90,

      kTilde = 126,

      k0 = 48,
      k1 = 49,
      k2 = 50,
      k3 = 51,
      k4 = 52,
      k5 = 53,
      k6 = 54,
      k7 = 55,
      k8 = 56,
      k9 = 57,

      kF1 = 0x70,
      kF2 = 0x71,
      kF3 = 0x72,
      kF4 = 0x73,
      kF5 = 0x74,
      kF6 = 0x75,
      kF7 = 0x76,
      kF8 = 0x77,
      kF9 = 0x78,
      kF10 = 0x79,
      kF11 = 0x7A,
      kF12 = 0x7B,
      kF13 = 0x7C,
      kF14 = 0x7D,
      kF15 = 0x7E,
      kF16 = 0x7F,
      kF17 = 0x80,
      kF18 = 0x81,
      kF19 = 0x82,
      kF20 = 0x83,
      kF21 = 0x84,
      kF22 = 0x85,
      kF23 = 0x86,
      kF24 = 0x87,
      kF25 = 0x88,

      kNum0 = 0x60,
      kNum1 = 0x61,
      kNum2 = 0x62,
      kNum3 = 0x63,
      kNum4 = 0x64,
      kNum5 = 0x65,
      kNum6 = 0x66,
      kNum7 = 0x67,
      kNum8 = 0x68,
      kNum9 = 0x69,

      kTab = 0x09,
      kBackspace = 0x08,
      kInsert = 0x2D,
      kDelete = 0x2E,
      kReturn = 0x0D,
      kEnter = (uint16_t)kReturn,
      kSpace = 0x20,
      kMinus = 0xBD,
      kShift = 0x10,
      kLShift = 0xA0,
      kRShift = 0xA1,
      kControl = 0x11,
      kLControl = 0xA2,
      kRControl = 0xA3,

      kLeft = 0x25,
      kUp = 0x26,
      kRight = 0x27,
      kDown = 0x28,
      
      kPageUp = 0x21,
      kPageDown = 0x22,
      kHome = 0x24,
      kEnd = 0x23,
      kCapsLock = 0x14,
      kScrollLock = 0x91,
      kNumLock = 0x90,
      kPrintScreen = 0x2C,
      kPause = 0x13,
      kDecimal = 0x6E,
      kDivide = 0x6F,
      kMultiply = 0x6A,
      kSubtract = 0x6D,
      kAdd = 0x6B,
      kEqual = 0xBB,
      kLeftBracket = 219,
      kRightBracket = 221,
      kBackSlash = 0xE2,
      kSemicolon = 186,
      kApostrophe = 0xDE,
      kGrave = (uint16_t)kTilde,
      kComma = 0xBC,
      kPeriod = 0xBE,
      kSlash = 191,
      kLAlt = 0x12,
      kLSuper = 0x5B,
      kRAlt = 0x12,
      kRSuper = 0x5C,
      kMenu = 0xA4,

      kEscape = 0x1B,
    };
  }
}


namespace eastl
{
  /////////////////////////////////////////////////////////////////////////////
  template <>
  struct hash<lambda::io::KeyboardKeys>
  {
    size_t operator()(const lambda::io::KeyboardKeys& k) const
    {
      return (size_t)k;
    }
  };
}

namespace lambda
{
  namespace io
  {
    ///////////////////////////////////////////////////////////////////////////
    class Keyboard
    {
    public:
      struct State
      {
      public:
        inline bool getKey(const KeyboardKeys& key) const
        {
          const auto& it = keys.find(key);
          if (it != keys.end())
            return it->second;

          return false;
        }
        inline void setKey(const KeyboardKeys& key, const bool& value)
        {
          const auto& it = keys.find(key);
          if (it != keys.end())
            it->second = value;
          else
            keys.insert(eastl::make_pair(key, value));
        }
      protected:
        UnorderedMap<KeyboardKeys, bool> keys;
      };
    };
  }
}
