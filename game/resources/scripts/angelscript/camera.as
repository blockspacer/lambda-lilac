#include "engine/components.as"
#include "engine/input_axis.as"
#include "engine/post_process.as"

class CameraData
{
  Vec3 rotation;
  Vec3 velocity;

  float inp_cam_hor;
  float inp_cam_ver;
  float inp_mov_hor;
  float inp_mov_ver;
  float inp_mov_spr;
  float inp_mov_jmp;
  float delta_time;
};

class FreeLookCamera
{
  void Initialize()
  {
    entity_base.Create();
    entity_camera.Create();

    entity_base.AddComponent(@transform_base);

    entity_camera.AddComponent(@transform_camera);
    entity_camera.AddComponent(@camera);
    transform_camera.SetLocalPosition(Vec3(0.0f, 0.75f, 0.0f));
    transform_camera.SetParent(@transform_base);
    Listener@ listener;
    entity_camera.AddComponent(@listener);
    listener.MakeMainListener();
    camera.SetFar(250.0f);

    Collider@ collider;
    entity_base.AddComponent(@collider);
    entity_base.AddComponent(@rigid_body);
    rigid_body.SetAngularConstraints(1 << 0 + 1 << 2);
    transform_base.SetLocalPosition(Vec3(0.0f, 20.0f, 0.0f));
    collider.MakeCapsuleCollider();

    // Handle shader passes.
    Array<String> input;
    
    // Normal.
    //Array<String> output_normal = { "normal", "depth_buffer" };
    //Asset::Shader deferred_shader_normal = asset_manager.LoadShader("resources/shaders/default_normal.fx");
    //camera.AddShaderPass("deferred_shader_normal", deferred_shader_normal, input, output_normal);
    
    // Pre-depth.
    //Array<String> output_pre_depth = { "depth_buffer" };
    //Asset::Shader deferred_shader_pre_depth = asset_manager.LoadShader("resources/shaders/default_pre-depth.fx");
    //camera.AddShaderPass("deferred_shader_pre_depth", deferred_shader_pre_depth, input, output_pre_depth);

    // Opaque.
    Array<String> output_opaque = { "albedo", "position", "normal", "metallic_roughness", "depth_buffer" };
    Asset::Shader deferred_shader_opaque = asset_manager.LoadShader("resources/shaders/default_opaque.fx");
    camera.AddShaderPass("deferred_shader_opaque", deferred_shader_opaque, input, output_opaque);
  }

  void FixedUpdate(const float&in fixed_delta_time)
  {
    // Get all data required to move the player / rotate the camera.
    CameraData data;
    data.delta_time  = fixed_delta_time;
    data.rotation    = rotation;
    data.velocity    = rigid_body.GetVelocity();
    data.inp_cam_hor = Input::GetAxis("camera_horizontal") * fixed_delta_time * sensitivity.x;
    data.inp_cam_ver = Input::GetAxis("camera_vertical")   * fixed_delta_time * sensitivity.x;
    data.inp_mov_hor = Input::GetAxis("movement_horizontal");
    data.inp_mov_ver = Input::GetAxis("movement_vertical");
    data.inp_mov_jmp = Input::GetAxis("movement_up_down");
    data.inp_mov_spr = Input::GetAxis("movement_sprint");

    // Free the player if its stuck.
    if(Input::GetAxis("free_me") > 0.0f)
    {
      transform_base.SetWorldPosition(transform_base.GetWorldPosition() + Vec3(0.0f, 5.0f, 0.0f));
    }

    // Get the new rotation and movement based on the data provided.
    RotateCamera(data);
    MoveCamera(data);

    // Apply the new rotation and velocity.
    rotation = data.rotation;
    transform_camera.SetLocalRotationEuler(Vec3(rotation.x, 0.0f, 0.0f) * Deg2Rad);
    transform_base.SetLocalRotationEuler(Vec3(0.0f, rotation.y, 0.0f) * Deg2Rad);
    rigid_body.SetVelocity(data.velocity);
  }

  Vec3 GetRotation() const
  {
    // Return the current rotation.
    return rotation;
  }
  Transform@ GetBaseTransform()
  {
    return transform_base;
  }
  Transform@ GetCameraTransform()
  {
    return transform_camera;
  }

  private void RotateCamera(CameraData&inout data)
  {
    // Rotate the camera.
    data.rotation  += Vec3(data.inp_cam_ver, data.inp_cam_hor, 0.0f);
    data.rotation.x = Clamp(-90.0f, 90.0f, data.rotation.x);
    data.rotation.y = Wrap(0.0f, 360.0f, data.rotation.y);
  }

  private void MoveCamera(CameraData&inout data)
  {
    // Do not allow the player to move if we're not on the ground.
    if(false == on_the_ground) return;

    // Jumping.
    bool last_held_jump = held_jump;
    held_jump = data.inp_mov_jmp > 0.0f;
    if(held_jump == true && last_held_jump == false)
    {
      data.velocity.y = Sqrt(2.0f * 9.81f * jump_height);
    }

    // Find out how badly we need to move.
    float sin = -Sin(Deg2Rad * data.rotation.y);
    float cos =  Cos(Deg2Rad * data.rotation.y);

    Vec2 movement(
      Vec2(+sin, -cos).Normalized() * data.inp_mov_ver +
      Vec2(-cos, -sin).Normalized() * data.inp_mov_hor
    );

    // Move the player.
    float movement_length = movement.LengthSqr();
    
		if (movement_length > 0.0f)
    {
      if (movement_length > 1.0f)
      {
        movement.Normalize();
      }
			
      float speed = speed_base + data.inp_mov_spr * (speed_sprint - speed_base);
      movement *= data.delta_time * speed;

      float vel_length = Vec2(data.velocity.x, data.velocity.z).Length();
      float mov_length = movement.Length();
			
      float mul = Clamp(0.0f, 1.0f, (Lerp(max_speed, max_speed_sprint, data.inp_mov_spr) - vel_length) / mov_length);
      data.velocity += Vec3(movement.x, 0.0f, movement.y) * mul;
    }
  }

  private void ModifyVelocity(CameraData&inout data)
  {
    // If we're on the ground then we should slow down immensly quickly.
    if(on_the_ground)
    {
      data.velocity.x *= 1.0f - data.delta_time * 2.0f;
      data.velocity.z *= 1.0f - data.delta_time * 2.0f;
    }

    // Add an extra umpfh to the fall of the jump.
    if(data.velocity.y < 0.0f)
    {
      data.velocity.y *= 1.0f + data.delta_time;
    }
  }

  private Entity entity_base;
  private Entity entity_camera;
  private Transform@ transform_base;
  private Transform@ transform_camera;
  private Camera@ camera;
  private RigidBody@ rigid_body;

  private Vec3 rotation;
  private float jump_height = 3.0f;
  private bool held_jump = false;
  private float speed_base = 7.5f;
  private float speed_sprint = 50.0f;
  private float max_speed = 10.0f;
  private float max_speed_sprint = 20.0f;
  private bool on_the_ground = true;
  private Vec2 sensitivity = Vec2(300.0f, 200.0f);
}
