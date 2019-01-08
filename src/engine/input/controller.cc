#include "controller.h"
#include <utils/console.h>

#if LAMBDA_WIN32
#include <Windows.h>
#include <Xinput.h>
#endif

namespace lambda
{
  namespace io
  {
    ///////////////////////////////////////////////////////////////////////////
    bool Controller::isConnected() const
    {
      return connected_;
    }

    ///////////////////////////////////////////////////////////////////////////
    void Controller::rumble(float lhs, float rhs) const
    {
#if LAMBDA_WIN32
      XINPUT_VIBRATION rumble{};
      rumble.wLeftMotorSpeed = (int)(lhs * 65535.0f);
      rumble.wRightMotorSpeed = (int)(rhs * 65535.0f);
      XInputSetState(id_,&rumble);
#endif
    }

    ///////////////////////////////////////////////////////////////////////////
    unsigned char Controller::getId() const
    {
      return id_;
    }

    ///////////////////////////////////////////////////////////////////////////
    bool Controller::axesInDeadzone(const Axes& axis) const
    {
#if LAMBDA_WIN32
      float value = axes_[(unsigned char)axis];

      switch (axis)
      {
      case Axes::kStickLX:
        return stickInDeadzone(
          value, 
          axes_[(unsigned char)Axes::kStickLY], 
          XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE
        );
        break;
      case Axes::kStickLY:
        return stickInDeadzone(
          axes_[(unsigned char)Axes::kStickLX],
          value, 
          XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE
        );
        break;
      case Axes::kStickRX:
        return stickInDeadzone(
          value, 
          axes_[(unsigned char)Axes::kStickRY], 
          XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE
        );
        break;
      case Axes::kStickRY:
        return stickInDeadzone(
          axes_[(unsigned char)Axes::kStickRX], 
          value, 
          XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE
        );
        break;
      case Axes::kTriggerL:
        return !(value > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) ? false : true;
        break;
      case Axes::kTriggerR:
        return !(value > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) ? false : true;
        break;
      }
#endif
      return false;
    }

    ///////////////////////////////////////////////////////////////////////////
    float Controller::getAxis(const Axes& axis) const
    {
      return axes_[(unsigned char)axis];
    }

    ///////////////////////////////////////////////////////////////////////////
    bool Controller::getButton(const Buttons& button) const
    {
      return buttons_[(unsigned char)button];
    }

    ///////////////////////////////////////////////////////////////////////////
    Controller::Controller(unsigned int id) :
      id_(id)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void Controller::update()
    {
#if LAMBDA_WIN32
      XINPUT_STATE state{};
      DWORD result = XInputGetState(id_,&state);

      // Is connected.
      connected_ = (result == ERROR_SUCCESS) ? true : false;

      // Buttons.
      for (unsigned char i = 0; i < (unsigned char)Buttons::kCount; ++i)
      {
        buttons_[i] = 
          (state.Gamepad.wButtons& buttonToXInputButton((Axes)i))
            > 0 ? true : false;
      }

      // Sticks.
      axes_[(unsigned char)Axes::kStickLX] = 
        (float)state.Gamepad.sThumbLX / 32768.0f;
      axes_[(unsigned char)Axes::kStickLY] = 
        (float)state.Gamepad.sThumbLY / 32768.0f;
      axes_[(unsigned char)Axes::kStickRX] = 
        (float)state.Gamepad.sThumbRX / 32768.0f;
      axes_[(unsigned char)Axes::kStickRY] = 
        (float)state.Gamepad.sThumbRY / 32768.0f;

      // Triggers.
      axes_[(unsigned char)Axes::kTriggerL] = 
        (float)state.Gamepad.bLeftTrigger / 255.0f;
      axes_[(unsigned char)Axes::kTriggerR] = 
        (float)state.Gamepad.bRightTrigger / 255.0f;
#endif
    }

    ///////////////////////////////////////////////////////////////////////////
    bool Controller::stickInDeadzone(float x, float y, int deadzone) const
    {
      x *= 32768.0f;
      y *= 32768.0f;
      return !(x > deadzone || x < -deadzone || y > deadzone || y < -deadzone);
    }

    ///////////////////////////////////////////////////////////////////////////
    unsigned short Controller::buttonToXInputButton(const Axes& axis) const
    {
#if LAMBDA_WIN32
      static const WORD buttonToXInputButton[] = {
        XINPUT_GAMEPAD_A,
        XINPUT_GAMEPAD_B,
        XINPUT_GAMEPAD_X,
        XINPUT_GAMEPAD_Y,
        XINPUT_GAMEPAD_DPAD_UP,
        XINPUT_GAMEPAD_DPAD_DOWN,
        XINPUT_GAMEPAD_DPAD_LEFT,
        XINPUT_GAMEPAD_DPAD_RIGHT,
        XINPUT_GAMEPAD_LEFT_SHOULDER,
        XINPUT_GAMEPAD_RIGHT_SHOULDER,
        XINPUT_GAMEPAD_LEFT_THUMB,
        XINPUT_GAMEPAD_RIGHT_THUMB,
        XINPUT_GAMEPAD_START,
        XINPUT_GAMEPAD_BACK
      };
      return buttonToXInputButton[(unsigned char)axis];
#endif
      return 0u;
    }

    ///////////////////////////////////////////////////////////////////////////
    const Controller& ControllerManager::getController(unsigned char id) const
    {
      assert(id < 4u);
      return controllers_[id];
    }

    ///////////////////////////////////////////////////////////////////////////
    void ControllerManager::update()
    {
      for (unsigned char i = 0u; i < 4u; ++i)
      {
        controllers_[i].update();
      }
    }
  }
}
