/** @file input.as */
#include "enums.as"

/**
* @defgroup Input Input
**/

namespace Input //! Namespace containing all input classes, enums and functions
{
    /**
    * @addtogroup Input
    * @{
    **/

    /**
    * @brief Check whether or not a key is pressed
    * @param key (const Input::Keys) The key that will be checked
    * @return (bool) Whether or not this key is pressed
    * @author Hilze Vonck
    **/
	bool IsKeyPressed(const Keys&in key)
	{
		return Violet_Input_Keyboard::IsKeyPressed(key);
	}
    /**
    * @brief Check whether or not a button is pressed
    * @param button (const Input::Buttons) The button that will be checked
    * @return (bool) Whether or not this button is pressed
    * @author Hilze Vonck
    **/
	bool IsButtonPressed(const Buttons&in button)
	{
		return Violet_Input_Keyboard::IsButtonPressed(button);
	}
    /**
    * @brief Get the value of an axis
    * @param axis (const Input::Axes) The axis of which the value will be retrieved
    * @return (float) The value of the axis
    * @author Hilze Vonck
    **/
	float GetAxis(const Axes&in axis)
	{
		return Violet_Input_Keyboard::GetAxis(axis);
	}

    /**
    * @}
    **/
}
