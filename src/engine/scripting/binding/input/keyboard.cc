#include <scripting/binding/input/keyboard.h>
#include <interfaces/iworld.h>
#include <input/controller.h>
#include <input/keyboard.h>
#include <input/mouse.h>

namespace lambda
{
  namespace scripting
  {
    namespace input
    {
      namespace keyboard
      {
        world::IWorld* g_world;

        bool IsKeyPressed(const uint16_t& key)
        {
          return g_world->getKeyboard().getCurrentState().getKey((io::KeyboardKeys)key);
        }
        bool IsButtonPressed(const uint8_t& button)
        {
          if (button < 3u)
            return g_world->getMouse().getCurrentState().getButton((io::MouseButtons)button);
          else if (g_world->getControllerManager().getController(0u).isConnected())
            return g_world->getControllerManager().getController(0u).getButton((io::Controller::Buttons)(button - 3u));
          else
            return false;
        }
        float GetAxis(const uint8_t& axis)
        {
          if (axis < (uint8_t)io::MouseAxes::kCount)
            return g_world->getMouse().getCurrentState().getAxis((io::MouseAxes)axis) - g_world->getMouse().getPreviousState().getAxis((io::MouseAxes)axis);
          else if (g_world->getControllerManager().getController(0u).isConnected() &&
            axis - (uint8_t)io::MouseAxes::kCount < (uint8_t)io::Controller::Axes::kCount)
          {
            io::Controller::Axes a = io::Controller::Axes(axis - (uint8_t)io::MouseAxes::kCount);
            const io::Controller& controller = g_world->getControllerManager().getController(0u);
            if (false == controller.axesInDeadzone(a))
            {
              float value = controller.getAxis(a);
              return (value * value * value);
            }
            else
              return 0.0f;
          }
          else
            return 0.0f;
        }

        extern Map<lambda::String, void*> Bind(world::IWorld* world)
        {
          g_world = world;

          return Map<lambda::String, void*>{
            { "bool Violet_Input_Keyboard::IsKeyPressed(const uint16& in)",   (void*)IsKeyPressed },
            { "bool Violet_Input_Keyboard::IsButtonPressed(const uint8& in)", (void*)IsButtonPressed },
            { "float Violet_Input_Keyboard::GetAxis(const uint8& in)",        (void*)GetAxis },
          };
        }
        void Unbind()
        {
          g_world = nullptr;
        }
      }
    }
  }
}
