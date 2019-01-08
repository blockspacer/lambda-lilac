#pragma once
#include <glm/glm.hpp>

namespace lambda
{
  namespace io
  {
    ///////////////////////////////////////////////////////////////////////////
    class Controller
    {
    public:
      /////////////////////////////////////////////////////////////////////////
      enum class Buttons
      {
        kA = 0,
        kB,
        kX,
        kY,
        kUp,
        kDown,
        kLeft,
        kRight,
        kBumperL,
        kBumperR,
        kStickL,
        kStickR,
        kStart,
        kBack,
        kCount
      };
      
      /////////////////////////////////////////////////////////////////////////
      enum class Axes
      {
        kStickLX = 0,
        kStickLY = 1,
        kStickRX = 2,
        kStickRY = 3,
        kTriggerL = 4,
        kTriggerR = 5,
        kCount
      };
      friend class ControllerManager;
    
    public:
      bool isConnected() const;
      void rumble(float lhs, float rhs) const;
      unsigned char getId() const;
      bool axesInDeadzone(const Axes& axis) const;
      float getAxis(const Axes& axis) const;
      bool getButton(const Buttons& button) const;

    protected:
      Controller(unsigned int id);
      void update();

    private:
      bool stickInDeadzone(float x, float y, int deadzone) const;
      unsigned short buttonToXInputButton(const Axes& axis) const;

    private:
      bool buttons_[(unsigned char)Buttons::kCount];
      float axes_[(unsigned char)Axes::kCount];
      unsigned char id_;
      bool connected_;
    };

    ///////////////////////////////////////////////////////////////////////////
    class ControllerManager
    {
    public:
      ControllerManager() {};
      const Controller& getController(unsigned char id) const;
      void update();

    private:
      Controller controllers_[4u] = {
        Controller(0u),
        Controller(1u),
        Controller(2u),
        Controller(3u)
      };
    };
  }
}
