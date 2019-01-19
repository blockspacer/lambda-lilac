#include "engine/input_axis.as"

class Inputter
{
  void RegisterInput()
  {
    RegisterInput_Camera();
    RegisterInput_Movement();
    RegisterInput_FlashLight();
  }

  private void RegisterInput_Movement()
  {
    // Register the axis for left / right movement.
    Input::InputAxis axis_movement_horizontal;
    //axis_movement_horizontal.AddState(Input::Helper::CreateAxis(Input::Axes::kStickLX, -1.0f));
    axis_movement_horizontal.AddState(Input::Helper::CreateKey(Input::Keys::kA,  1.0f));
    axis_movement_horizontal.AddState(Input::Helper::CreateKey(Input::Keys::kD, -1.0f));
    Input::RegisterAxis("movement_horizontal", axis_movement_horizontal);
    
    // Register the axis for forward / backward movement.
    Input::InputAxis axis_movement_vertical;
    //axis_movement_vertical.AddState(Input::Helper::CreateAxis(Input::Axes::kStickLY, 1.0f));
    axis_movement_vertical.AddState(Input::Helper::CreateKey(Input::Keys::kW,  1.0f));
    axis_movement_vertical.AddState(Input::Helper::CreateKey(Input::Keys::kS, -1.0f));
    Input::RegisterAxis("movement_vertical", axis_movement_vertical);
    
    // Register the axis for sprinting.
    Input::InputAxis axis_movement_sprint;
    //axis_movement_sprint.AddState(Input::Helper::CreateAxis(Input::Axes::kTriggerR, 1.0f));
    axis_movement_sprint.AddState(Input::Helper::CreateKey(Input::Keys::kShift, 1.0f));
    Input::RegisterAxis("movement_sprint", axis_movement_sprint);

    // Register the axis for jumping / ducking.
    Input::InputAxis axis_movement_up_down;
    //axis_movement_up_down.AddState(Input::Helper::CreateButton(Input::Buttons::kY, -1.0f));
    //axis_movement_up_down.AddState(Input::Helper::CreateButton(Input::Buttons::kA,  1.0f));
    axis_movement_up_down.AddState(Input::Helper::CreateKey(Input::Keys::kQ, -1.0f));
    axis_movement_up_down.AddState(Input::Helper::CreateKey(Input::Keys::kE,  1.0f));
    Input::RegisterAxis("movement_up_down", axis_movement_up_down);
    
    // Register the axis for freeing the player.
    Input::InputAxis axis_free_me;
    axis_free_me.AddState(Input::Helper::CreateKey(Input::Keys::kP, 1.0f));
    Input::RegisterAxis("free_me", axis_free_me);
  }
  
  private void RegisterInput_Camera()
  {
    // TODO (Hilze): Add mouse support.

    // register the camera left / right rotation.
    Input::InputAxis axis_camera_horizontal;
    //axis_camera_horizontal.AddState(Input::Helper::CreateAxis(Input::Axes::kStickRX, -1.0f));
    axis_camera_horizontal.AddState(Input::Helper::CreateKey(Input::Keys::kLeft, 1.0f));
    axis_camera_horizontal.AddState(Input::Helper::CreateKey(Input::Keys::kRight, -1.0f));
    Input::RegisterAxis("camera_horizontal", axis_camera_horizontal);
    
    // register the camera up / down rotation.
    Input::InputAxis axis_camera_vertical;
    //axis_camera_vertical.AddState(Input::Helper::CreateAxis(Input::Axes::kStickRY, 1.0f));
    axis_camera_vertical.AddState(Input::Helper::CreateKey(Input::Keys::kUp,    1.0f));
    axis_camera_vertical.AddState(Input::Helper::CreateKey(Input::Keys::kDown, -1.0f));
    Input::RegisterAxis("camera_vertical", axis_camera_vertical);
  }

  private void RegisterInput_FlashLight()
  {
    // Register the flash light enable/disable axis.
    Input::InputAxis axis_flash_light;
    //axis_flash_light.AddState(Input::Helper::CreateButton(Input::Buttons::kUp, 1.0f));
    axis_flash_light.AddState(Input::Helper::CreateKey(Input::Keys::kF, 1.0f));
    Input::RegisterAxis("flash_light", axis_flash_light);
  }
}