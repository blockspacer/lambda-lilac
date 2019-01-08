#pragma once
#include "input.h"
#include <glm/glm.hpp>
#include <containers/containers.h>

namespace lambda
{
  namespace io
  {
    ///////////////////////////////////////////////////////////////////////////
    enum class MouseButtons
    {
      kLMB = 0,
      kMMB = 1,
      kRMB = 2,
      kLMBD = 3,
      kMMBD = 4,
      kRMBD = 5,
    };

    ///////////////////////////////////////////////////////////////////////////
    enum class MouseAxes
    {
      kMouseX = 0,
      kMouseY = 1,
      kScroll = 2,
      kCount
    };
  }
}

namespace eastl
{
  /////////////////////////////////////////////////////////////////////////////
  template <>
  struct hash<lambda::io::MouseAxes>
  {
    size_t operator()(const lambda::io::MouseAxes& k) const
    {
      return (size_t)k;
    }
  };
  
  /////////////////////////////////////////////////////////////////////////////
  template <>
  struct hash<lambda::io::MouseButtons>
  {
    size_t operator()(const lambda::io::MouseButtons& k) const
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
    class Mouse
    {
    public:
      /////////////////////////////////////////////////////////////////////////
      struct State
      {
      public:
        inline bool getButton(const MouseButtons& button) const
        {
          const auto& it = buttons.find(button);
          if (it != buttons.end())
            return it->second;

          return false;
        }
        inline void setButton(const MouseButtons& button, const bool& value)
        {
          const auto& it = buttons.find(button);
          if (it != buttons.end())
            it->second = value;
          else
            buttons.insert(eastl::make_pair(button, value));
        }
        inline float getAxis(const MouseAxes& axis) const
        {
          return axes[(unsigned char)axis];
        }
        inline void setAxis(const MouseAxes& axis, const float& value)
        {
          axes[(unsigned char)axis] = value;
        }

      private:
        float axes[(unsigned char)MouseAxes::kCount];
        UnorderedMap<MouseButtons, bool> buttons;
      };
    };
  }
}
