#include "engine/asset_manager.as"
#include "engine/input_axis.as"
#include "post_process.as"
#include "engine/graphics.as"
#include "input.as"
#include "camera.as"
#include "lighting.as"
#include "ground.as"
#include "trees.as"
#include "world.as"
//#include "character.as"

class Game
{
  void Initialize()
  {
    // Create the character.
    //character.Create();

    // Initialize the ground.
    //ground.ZeroHeight();
    ground.Initialize();

    // Spawn the trees.
    //trees.Disable();
    trees.Initialize(ground);

    // Handle the world.
    world.Disable();
    //world.AddModel("resources/gltf/sponza.glb");
    //world.AddModel("resources/gltf/san_miguel_low_poly.glb");
    world.AddModel("resources/gltf/metal_roughness/MetalRoughSpheres.gltf");

    // Initialize all post processing effects.
    SetShaderVariable("water_height", ground.GetWaterHeight());
    post_process.InitializePostProcessing();

    // Initialize the camera.
    camera.Initialize();

    // Initialize the lighting.
    lighting.InitializeLighting();

    // Initialize the input.
    inputter.RegisterInput();

    // Move the camera so you do not spawn under the ground.
    Vec3 new_position = camera.GetBaseTransform().GetWorldPosition();
    new_position.y = ground.HeightOnPosition(new_position) + 2.0f;
    camera.GetBaseTransform().SetWorldPosition(new_position);
  }

  void Terminate()
  {
  }

  void Update(const float delta_time)
  {
    if (init_once)
    {
      init_once = false;
      //Asset::Shader shader = asset_manager.LoadShader("resources/shaders/apply_gui.fx");
      //Array<String> input  = { "post_process_buffer", "gui" };
      //Array<String> output = { "post_process_buffer" };
      //AddShaderPass("apply_gui", shader, input, output);
    }

    // Update the character.
    //character.Update(delta_time);

    // Often used variables.
    Transform@ transform;

    // Update the RSM light.
    float sin_y_rot = -Sin(Deg2Rad * camera.GetRotation().y);
    float cos_y_rot = -Cos(Deg2Rad * camera.GetRotation().y);
    Vec3 offset = Vec3(sin_y_rot, 0.0f, cos_y_rot).Normalized() * 25.0f;
    lighting.GetRSM().GetComponent(@transform);
    transform.SetWorldPosition(camera.GetBaseTransform().GetWorldPosition() + offset);

    return;

    // Set the rotation of the lights (front and back).
    Vec3 forward = Vec3(0.0f, 1.0f, 0.25f).Normalized();
    lighting.GetSunFront().GetComponent(@transform);
    transform.SetWorldRotation(
      LookRotation(
        forward,
        Vec3(0.0f, 1.0f, 0.0f)
      )
    );

    lighting.GetSunBack().GetComponent(@transform);
    transform.SetWorldRotation(
      LookRotation(
        -forward,
        Vec3(0.0f, 1.0f, 0.0f)
      )
    );

    return;

    // Update flash light.
    lighting.GetFlashLight().GetComponent(@transform);
    transform.SetWorldRotationEuler((camera.GetRotation() + Vec3(180.0f, 0.0f, 0.0f)) * Deg2Rad);
    transform.SetWorldPosition(camera.GetBaseTransform().GetWorldPosition());
    transform.SetWorldPosition(Vec3(0.0f, 5.0f, 0.0f));
    transform.SetWorldRotationEuler(Vec3(0.0f, 0.0f, 0.0f) * Deg2Rad);

    // Enable or disable the flash light.
    bool last_flashlight_value = flashlight_value;
    flashlight_value = Input::GetAxis("flash_light") > 0.0f;
    if(flashlight_value == true && last_flashlight_value == false)
    {
      // Enable / disable the spot light.
      SpotLight@ spot_light;
      lighting.GetFlashLight().GetComponent(@spot_light);
      spot_light.SetEnabled(!spot_light.GetEnabled());

      // Play the enable / disable sound with a random pitch to keep it from annoying the player.
      WaveSource@ wave_source;
      lighting.GetFlashLight().GetComponent(@wave_source);
      wave_source.SetPitch(Random(0.9f, 1.1f));
      wave_source.Play();
    }

    // Update water shader.
    bool water_shader_enabled = (camera.GetCameraTransform().GetWorldPosition().y < ground.GetWaterHeight());
    if(last_water_shader_value != water_shader_enabled)
    {
      last_water_shader_value = water_shader_enabled;
      SetShaderPassEnabled("water", water_shader_enabled);
    }
  }

  void FixedUpdate(const float fixed_delta_time)
  {
    // Update the camera.
    camera.FixedUpdate(fixed_delta_time);
  }

  //private Character character;
  private PostProcesser post_process;
  private Lighting lighting;
  private Inputter inputter;
  private FreeLookCamera camera;

  private Ground ground;
  private Trees trees;
  private World world;

  private bool init_once               = true;
  private bool flashlight_value        = false;
  private bool last_water_shader_value = true;
}
