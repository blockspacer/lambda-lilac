#include "input.as"

namespace Input //! Input
{
	/**
	* @addtogroup Input
	* @{
	**/

	/**
	* @class Input::InputState
	* @brief A base class which should not directly be instanciated
	* @author Hilze Vonck
	**/
	class InputState
	{
		/**
		* @brief Returns the value of this input state if it is not blocked by any of its blockers 
		* @return (float) The value of this input state
		* @public
		**/
		float GetValue()
		{
			for(uint8 i = 0; i < blockers.Size(); ++i)
			{
				if(blockers[i].GetValue() == 0.0f)
				{
					return 0.0f;
				}
			}

			return GetRawValue();
		}
		/**
		* @brief Adds a blocker to this input states list of blockers 
		* @param blocker (Input:InputState@) A pointer to a blocker
		* @public
		**/
		void AddBlocker(InputState@ blocker)
		{
			blockers.PushBack(blocker);
		}
		/**
		* @brief Sets the sensitivity of this input state. Only directly used by axes. For buttons and keys it is the return value 
		* @param sensitivity (const float) The new sensitivity
		* @public
		**/
		void SetSensitivity(const float&in sensitivity)
		{
			this.sensitivity = sensitivity;
		}
		/**
		* @brief Gets the sensitivity of this input state. Only directly used by axes. For buttons and keys it is the return value 
		* @return (float) The sensitivity
		* @public
		**/
		float GetSensitivity() const
		{
			return sensitivity;
		}
		/**
		* @brief Gets the raw value of this input state. This is overwritten by the inherited classes
		* @return (float) The raw value
		* @public
		**/
		protected float GetRawValue() const
		{
			return 0.0f;
		}


		private Array<InputState@> blockers; //!< This input states blockers
		protected float sensitivity = 1.0f; //!< The sensitivity of this input state
	};

	/**
	* @class Input::InputStateKey
	* @brief An input state for getting keyboard input
	* @author Hilze Vonck
	**/
	class InputStateKey : InputState
	{
		/**
		* @brief Sets the key that this state should check
		* @param key (const Input::Keys) The new key
		* @public
		**/
		void SetKey(const Keys&in key)
		{
			this.key = key;
		}
		/**
		* @brief Gets the key that this state should check
		* @return (Input::Keys) The key
		* @public
		**/
		Keys Getkey() const
		{
			return key;
		}
		/**
		* @brief Set whether or not this key should be pressed
		* @param should_be_pressed (const bool) Should this key be pressed?
		* @public
		**/
		void SetShouldBePressed(const bool&in should_be_pressed)
		{
			this.should_be_pressed = should_be_pressed;
		}
		/**
		* @brief Get whether or not this key should be pressed
		* @return (bool) Should this key be pressed?
		* @public
		**/
		bool GetShouldBePressed() const
		{
			return should_be_pressed;
		}
		/**
		* @brief Returns the raw value of this key. Returns either zero or sensititivity
		* @return (float) The raw value
		* @public
		**/
		protected float GetRawValue() const
		{
			bool is_pressed = IsKeyPressed(key);

			if(is_pressed != should_be_pressed)
			{
				return 0.0f;
			}

			return sensitivity;
		}

		private Keys key; //!< The key that it should check
		private bool should_be_pressed = true; //!< Should this key be pressed or not?
	};

	/**
	* @class Input::InputStateButton
	* @brief An input state for getting controller button or mouse button input
	* @author Hilze Vonck
	**/
	class InputStateButton : InputState
	{
		/**
		* @brief Sets the button that this state should check
		* @param button (const Input::Buttons) The new button
		* @public
		**/
		void SetButton(const Buttons&in button)
		{
			this.button = button;
		}
		/**
		* @brief Gets the button that this state should check
		* @return (Input::Buttons) The button
		* @public
		**/
		Buttons GetButton() const
		{
			return button;
		}
		/**
		* @brief Set whether or not this button should be pressed
		* @param should_be_pressed (const bool) Should this button be pressed?
		* @public
		**/
		void SetShouldBePressed(const bool&in should_be_pressed)
		{
			this.should_be_pressed = should_be_pressed;
		}
		/**
		* @brief Get whether or not this button should be pressed
		* @return (bool) Should this button be pressed?
		* @public
		**/
		bool GetShouldBePressed() const
		{
			return should_be_pressed;
		}
		/**
		* @brief Returns the raw value of this button. Returns either zero or sensititivity
		* @return (float) The raw value
		* @public
		**/
		protected float GetRawValue() const
		{
			bool is_pressed = IsButtonPressed(button);

			if(is_pressed != should_be_pressed)
			{
				return 0.0f;
			}

			return sensitivity;
		}

		private Buttons button; //!< The button that it should check
		private bool should_be_pressed = true; //!< Should this button be pressed or not?
	}

	/**
	* @class Input::InputStateAxis
	* @brief An input state for getting controller axis or mouse axis input
	* @author Hilze Vonck
	**/
	class InputStateAxis : InputState
	{
		/**
		* @brief Sets the axis that this state should check
		* @param axis (const Input::Axes) The new axis
		* @public
		**/
		void SetAxis(const Axes&in axis)
		{
			this.axis = axis;
		}
		/**
		* @brief Gets the axis that this state should check
		* @return (Input::Axes) The axis
		* @public
		**/
		Axes GetAxis() const
		{
			return axis;
		}
		/**
		* @brief Set whether or not this axis should be zero
		* @param should_be_zero (const bool) Should this axis be zero?
		* @public
		**/
		void SetShouldBeZero(const bool&in should_be_zero)
		{
			this.should_be_zero = should_be_zero;
		}
		/**
		* @brief Get whether or not this axis should be zero
		* @return (bool) Should this axis be zero?
		* @public
		**/
		bool GetShouldBeZero() const
		{
			return should_be_zero;
		}
		/**
		* @brief Returns the raw value of this axis mutiplied by the sensititivity
		* @return (float) The raw value
		* @public
		**/
		protected float GetRawValue() const
		{
			float value = Input::GetAxis(axis);
			
      if (should_be_zero)
      {
				if (value != 0.0f)
				{
					return 0.0f;
				}
				else
				{
					return sensitivity;
				}
			}

      return value * sensitivity;
		}

		private Axes axis; //!< The axis that it should check
		private bool should_be_zero = false; //!< Should this axis be zero or not?
	}

	namespace Helper //! A helper namespace for the input. Makes it easier for you to create button, key and axis states
	{
		/**
		* @brief Creates an InputStateKey for you based upon a few parameters
		* @param key (const Input::Keys) The key
		* @param sensitivity (const float) The sensitivity
		* @param should_be_pressed (const bool) Should the button be pressed?
		* @param blockers (Array<InputState@>) What are the blockers?
		* @return InputStateKey The generated input state
		* @author Hilze Vonck
		**/
		InputStateKey CreateKey(const Keys&in key, const float&in sensitivity = 1.0f, const bool&in should_be_pressed = true, const Array<InputState@>&in blockers = Array<InputState@>())
    {
			InputStateKey state;
			state.SetKey(key);
			state.SetSensitivity(sensitivity);
			state.SetShouldBePressed(should_be_pressed);
			for (uint8 i = 0; i < blockers.Size(); ++i)
			{
				state.AddBlocker(blockers[i]);
			}
			return state;
    }
		  /**
		* @brief Creates an InputStateAxis for you based upon a few parameters
		* @param axis (const Input::Axes) The axis
		* @param sensitivity (const float) The sensitivity
		* @param should_be_zero (const bool) Should the axis be zero?
		* @param blockers (Array<InputState@>) What are the blockers?
		* @return InputStateAxis The generated input state
		* @author Hilze Vonck
		**/
		InputStateAxis CreateAxis(const Axes&in axis, const float&in sensitivity = 1.0f, const bool&in should_be_zero = false, const Array<InputState@>&in blockers = Array<InputState@>())
    {
			InputStateAxis state;
			state.SetAxis(axis);
			state.SetSensitivity(sensitivity);
			state.SetShouldBeZero(should_be_zero);
			for (uint8 i = 0; i < blockers.Size(); ++i)
			{
				state.AddBlocker(blockers[i]);
			}
			return state;
    }
		/**
		* @brief Creates an InputStateButton for you based upon a few parameters
		* @param button (const Input::Buttons) The button
		* @param sensitivity (const float) The sensitivity
		* @param should_be_pressed (const bool) Should the button be pressed?
		* @param blockers (Array<InputState@>) What are the blockers?
		* @return InputStateButton The generated input state
		* @author Hilze Vonck
		**/
		InputStateButton CreateButton(const Buttons&in button, const float&in sensitivity = 1.0f, const bool&in should_be_pressed = true, const Array<InputState@>&in blockers = Array<InputState@>())
    {
			InputStateButton state;
			state.SetButton(button);
			state.SetSensitivity(sensitivity);
			state.SetShouldBePressed(should_be_pressed);
			for (uint8 i = 0; i < blockers.Size(); ++i)
			{
				state.AddBlocker(blockers[i]);
			}
			return state;
    }
	}

	/**
	* @class Input::InputAxis
	* @brief An input axis which takes multiple input states. Makes it easy for you to support both keyboard and mouse and controller
	* @author Hilze Vonck
	**/
	class InputAxis
	{
		/**
		* @brief Adds a state to the axis. This state will contribute to the axis total value
		* @param state (Input::InputState@) A pointer to an input state
		* @public
		**/
		void AddState(InputState@ state)
		{
			input_states.PushBack(state);
		}
		/**
		* @brief Returns the combined value of this input axis's input states 
		* @return (float) The combined value
		* @public
		**/
		float GetValue() const
		{
			float value = 0.0f;

			for(uint8 i = 0; i < input_states.Size(); ++i)
			{
				value += input_states[i].GetValue();
			}
			return value;
		}

		private Array<InputState@> input_states; //!< All input states that this input axis will use to create the value
	}

	/**
	* @cond HIDDEN_SYMBOLS
	**/
	namespace InputHelper
	{
		class Axis
		{
			String name; //!< The name of the axis
			InputAxis axis; //!< The actual axis
			float value; //!< The last value of the axis
		}
		Array<Axis> axes; //!< All registered axes

		/**
		* @brief Updates all values of the registered axes 
		* @author Hilze Vonck
		**/
		void UpdateAxes()
		{
			for(uint8 i = 0; i < axes.Size(); ++i)
			{
				axes[i].value = axes[i].axis.GetValue();
			}
		}
	}
	/**
	* @endcond
	**/

	/**
	* @brief Registers a copy of an axis under the provided name. The used input axis can be discarted afterwards 
	* @param name (const String) The name of the axis
	* @param axis (const Input::InputAxis&) The axis
	* @author Hilze Vonck
	**/
	void RegisterAxis(const String&in name, const InputAxis&in axis)
	{
		Input::InputHelper::Axis a;
		a.name  = name;
		a.axis  = axis;
		a.value = 0.0f;
		Input::InputHelper::axes.PushBack(a);
	}
	/**
	* @brief Returns the value of a registered axis 
	* @param name (const String) The name of the axis
	* @return (float) The value of the axis
	* @author Hilze Vonck
	**/
	float GetAxis(const String&in name)
	{
		for(uint8 i = 0; i < Input::InputHelper::axes.Size(); ++i)
		{
			if(Input::InputHelper::axes[i].name == name)
			{
				return Input::InputHelper::axes[i].value;
			}
		}
		return 0.0f;
	}

	/**
	* @}
	**/
}