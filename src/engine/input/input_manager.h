#pragma once
#include "controller.h"
#include "mouse.h"
#include "keyboard.h"
#include <containers/containers.h>
#include <memory/memory.h>

namespace lambda
{
  namespace io
  {
    ///////////////////////////////////////////////////////////////////////////
    struct InputStateData
    {
      const Input<Keyboard::State>& keyboard;
      const Input<Mouse::State>& mouse;
      const ControllerManager& controller_manager;
    };

    ///////////////////////////////////////////////////////////////////////////
    class InputState
    {
    public:
      virtual ~InputState() {}
      float getValue(const InputStateData& input)
      {
        bool valid = true;
        for (foundation::SharedPointer<InputState> blocker : blockers)
        {
          if (blocker->getValue(input) == 0.0f)
          {
            valid = false;
            break;
          }
        }
        if (valid)
          return getRawValue(input);
        else
          return 0.0f;
      }
      void AddBlocker(foundation::SharedPointer<InputState> blocker)
      {
        blockers.push_back(blocker);
      }

    protected:
      virtual float getRawValue(const InputStateData& input) = 0;

    private:
      Vector<foundation::SharedPointer<InputState>> blockers;
    };
    
    ///////////////////////////////////////////////////////////////////////////
    class InputStateKeyboard : public InputState
    {
    public:
      virtual ~InputStateKeyboard() {}
      void setKey(const KeyboardKeys& key)
      {
        key_ = key;
      }
      void setShouldBePressed(const bool& pressed)
      {
        pressed_ = pressed;
      }
      void setSensitivity(const float& sensitivity)
      {
        sensitivity_ = sensitivity;
      }
      KeyboardKeys getKey() const
      {
        return key_;
      }
      bool getShouldBePressed() const
      {
        return pressed_;
      }
      float getSensitivity() const
      {
        return sensitivity_;
      }
    protected:
      virtual float getRawValue(const InputStateData& input) override
      {
        bool is_pressed = input.keyboard.getCurrentState().getKey(key_);

        if (is_pressed != pressed_)
          return 0.0f;

        return sensitivity_;
      }

    private:
      KeyboardKeys key_;
      bool pressed_ = true;
      float sensitivity_ = 1.0f;
    };
   
    ///////////////////////////////////////////////////////////////////////////
    class InputStateMouseButton : public InputState
    {
    public:
      virtual ~InputStateMouseButton() {}
      void setButton(const MouseButtons& button)
      {
        button_ = button;
      }
      void setShouldBePressed(const bool& pressed)
      {
        pressed_ = pressed;
      }
      void setSensitivity(const float& sensitivity)
      {
        sensitivity_ = sensitivity;
      }
      MouseButtons getButton() const
      {
        return button_;
      }
      bool getShouldBePressed() const
      {
        return pressed_;
      }
      float getSensitivity() const
      {
        return sensitivity_;
      }
    protected:
      virtual float getRawValue(const InputStateData& input) override
      {
        bool is_pressed = input.mouse.getCurrentState().getButton(button_);

        if (is_pressed != pressed_)
          return 0.0f;

        return sensitivity_;
      }

    private:
      MouseButtons button_;
      bool pressed_ = true;
      float sensitivity_ = 1.0f;
    };
    
    ///////////////////////////////////////////////////////////////////////////
    class InputStateMouseAxis : public InputState
    {
    public:
      virtual ~InputStateMouseAxis() {}
      void setAxis(const MouseAxes& axis)
      {
        axis_ = axis;
      }
      void setShouldBeZero(const bool& zero)
      {
        zero_ = zero;
      }
      void setSensitivity(const float& sensitivity)
      {
        sensitivity_ = sensitivity;
      }
      MouseAxes getAxis() const
      {
        return axis_;
      }
      bool getShouldBeZero() const
      {
        return zero_;
      }
      float getSensitivity() const
      {
        return sensitivity_;
      }
    protected:
      virtual float getRawValue(const InputStateData& input) override
      {
        float value = 
          input.mouse.getCurrentState().getAxis(axis_) - 
          input.mouse.getPreviousState().getAxis(axis_);

        if (zero_)
        {
          if (value != 0.0f)
            return 0.0f;
          else
            return sensitivity_;
        }

        return value * sensitivity_;
      }

    private:
      MouseAxes axis_;
      bool zero_ = false;
      float sensitivity_ = 1.0f;
    };
   
    ///////////////////////////////////////////////////////////////////////////
    class InputStateControllerButton : public InputState
    {
    public:
      virtual ~InputStateControllerButton() {}
      void setButton(const Controller::Buttons& button)
      {
        button_ = button;
      }
      void setControllerId(const unsigned char& id)
      {
        id_ = id;
      }
      void setShouldBePressed(const bool& pressed)
      {
        pressed_ = pressed;
      }
      void setSensitivity(const float& sensitivity)
      {
        sensitivity_ = sensitivity;
      }
      Controller::Buttons getButton() const
      {
        return button_;
      }
      unsigned char getControllerId() const
      {
        return id_;
      }
      bool getShouldBePressed() const
      {
        return pressed_;
      }
      float getSensitivity() const
      {
        return sensitivity_;
      }
    protected:
      virtual float getRawValue(const InputStateData& input) override
      {
        bool is_pressed = 
          input.controller_manager.getController(id_).getButton(button_);

        if (is_pressed != pressed_)
          return 0.0f;

        return sensitivity_;
      }

    private:
      unsigned char id_ = 0u;
      Controller::Buttons button_;
      bool pressed_ = true;
      float sensitivity_ = 1.0f;
    };
  
    ///////////////////////////////////////////////////////////////////////////
    class InputStateControllerAxis : public InputState
    {
    public:
      virtual ~InputStateControllerAxis() {}
      void setAxis(const Controller::Axes& axis)
      {
        axis_ = axis;
      }
      void setControllerId(const unsigned char& id)
      {
        id_ = id;
      }
      void setShouldBeZero(const bool& zero)
      {
        zero_ = zero;
      }
      void setSensitivity(const float& sensitivity)
      {
        sensitivity_ = sensitivity;
      }
      Controller::Axes getAxis() const
      {
        return axis_;
      }
      unsigned char getControllerId() const
      {
        return id_;
      }
      bool getShouldBeZero() const
      {
        return zero_;
      }
      float getSensitivity() const
      {
        return sensitivity_;
      }
    protected:
      virtual float getRawValue(const InputStateData& input) override
      {
        const Controller& controller = 
          input.controller_manager.getController(id_);

        float value = 
          controller.axesInDeadzone(axis_) ? 0.0f : controller.getAxis(axis_);

        if (zero_)
        {
          if (value != 0.0f)
            return 0.0f;
          else
            return sensitivity_;
        }

        return value * sensitivity_;
      }

    private:
      unsigned char id_ = 0;
      Controller::Axes axis_;
      bool zero_ = false;
      float sensitivity_ = 1.0f;
    };

    ///////////////////////////////////////////////////////////////////////////
    class InputAxis
    {
    public:
      virtual ~InputAxis() {}
      void addState(foundation::SharedPointer<InputState> state)
      {
        states_.push_back(state);
      }
      float getValue(const InputStateData& input) const
      {
        float value = 0.0f;

        for (foundation::SharedPointer<InputState> state : states_)
        {
          value += state->getValue(input);
        }

        return value;
      }

    private:
      Vector<foundation::SharedPointer<InputState>> states_;
    };

    ///////////////////////////////////////////////////////////////////////////
    class InputStateHelper
    {
    public:
      static foundation::SharedPointer<io::InputStateKeyboard> keyboard(
        const io::KeyboardKeys& key, 
        float sensitivity = 1.0f, 
        bool should_be_pressed = true, 
        const Vector<foundation::SharedPointer<InputState>>& blockers = {})
      {
        foundation::SharedPointer<io::InputStateKeyboard> state = 
          foundation::Memory::constructShared<io::InputStateKeyboard>();
        state->setKey(key);
        state->setSensitivity(sensitivity);
        state->setShouldBePressed(should_be_pressed);
        for (const auto& blocker : blockers) state->AddBlocker(blocker);
        return state;
      }
      static foundation::SharedPointer<io::InputStateControllerAxis> 
        controllerAxis(
          const unsigned char& id, 
          const io::Controller::Axes& axis, 
          float sensitivity = 1.0f, 
          bool zero = false, 
          const Vector<foundation::SharedPointer<InputState>>& blockers = {})
      {
        foundation::SharedPointer<io::InputStateControllerAxis> state = 
          foundation::Memory::constructShared<io::InputStateControllerAxis>();
        state->setControllerId(id);
        state->setAxis(axis);
        state->setSensitivity(sensitivity);
        state->setShouldBeZero(zero);
        for (const auto& blocker : blockers) state->AddBlocker(blocker);
        return state;
      }
      static foundation::SharedPointer<io::InputStateControllerButton> 
        controllerButton(
          const unsigned char& id, 
          const io::Controller::Buttons& button, 
          float sensitivity = 1.0f, 
          bool should_be_pressed = true, 
          const Vector<foundation::SharedPointer<InputState>>& blockers = {})
      {
        foundation::SharedPointer<io::InputStateControllerButton> state = 
         foundation::Memory::constructShared<io::InputStateControllerButton>();
        state->setControllerId(id);
        state->setButton(button);
        state->setSensitivity(sensitivity);
        state->setShouldBePressed(should_be_pressed);
        for (const auto& blocker : blockers) state->AddBlocker(blocker);
        return state;
      }
      static foundation::SharedPointer<io::InputStateMouseAxis> mouseAxis(
        const io::MouseAxes& axis, 
        float sensitivity = 1.0f, 
        bool zero = false, 
        const Vector<foundation::SharedPointer<InputState>>& blockers = {})
      {
        foundation::SharedPointer<io::InputStateMouseAxis> state = 
          foundation::Memory::constructShared<io::InputStateMouseAxis>();
        state->setAxis(axis);
        state->setSensitivity(sensitivity);
        state->setShouldBeZero(zero);
        for (const auto& blocker : blockers) state->AddBlocker(blocker);
        return state;
      }
      static foundation::SharedPointer<io::InputStateMouseButton> mouseButton(
        const io::MouseButtons& button, 
        float sensitivity = 1.0f, 
        bool should_be_pressed = true, 
        const Vector<foundation::SharedPointer<InputState>>& blockers = {})
      {
        foundation::SharedPointer<io::InputStateMouseButton> state = 
          foundation::Memory::constructShared<io::InputStateMouseButton>();
        state->setButton(button);
        state->setSensitivity(sensitivity);
        state->setShouldBePressed(should_be_pressed);
        for (const auto& blocker : blockers) state->AddBlocker(blocker);
        return state;
      }
    };

    ///////////////////////////////////////////////////////////////////////////
    class InputManager
    {
    public:
      InputManager(
        const Input<Keyboard::State>& keyboard,
        const Input<Mouse::State>& mouse,
        const ControllerManager& controller_manager
      ) :
        keyboard_(keyboard),
        mouse_(mouse),
        controller_manager_(controller_manager)
      {}
      inline void registerAxis(const String& name, const InputAxis& axis)
      {
        registered_axis_.insert(eastl::make_pair(name, axis));
      }
      float getAxis(const String& name)
      {
        const auto& it = registered_axis_.find(name);
        if (it != registered_axis_.end())
        {
          InputStateData input = {
            keyboard_,
            mouse_,
            controller_manager_
          };

          return it->second.getValue(input);
        }
        return 0.0f;
      }

    private:
      UnorderedMap<String, InputAxis> registered_axis_;

      const Input<Keyboard::State>& keyboard_;
      const Input<Mouse::State>& mouse_;
      const ControllerManager& controller_manager_;
    };
  }
}
