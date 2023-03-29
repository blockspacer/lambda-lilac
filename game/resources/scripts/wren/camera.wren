import "Core" for Vec2, Vec3, Vec4
import "Core" for Texture, Shader, Mesh
import "Core" for Console
import "Core" for GameObject, Transform, Camera, Lod, RigidBody, PhysicsConstraints, WaveSource, Collider, MonoBehaviour
import "Core" for Physics, Manifold
import "Core" for Input, Keys, Buttons, Axes
import "Core" for Math, Time, Debug, Sort, PostProcess
import "Core" for Light, LightTypes, ShadowTypes

import "resources/scripts/wren/input_controller" for InputController
import "resources/scripts/wren/shuriken"         for ShurikenBehaviour
import "resources/scripts/wren/physics_layers"   for PhysicsLayers

class Sorter {
  construct new(pos) {  _pos = pos                                                 }
  greater(lhs, rhs)  { (_pos - lhs.point).lengthSqr > (_pos - rhs.point).lengthSqr }
}

class FreeLookCamera is MonoBehaviour {
  construct new()      { super()                     }
  static goGet(val)    { MonoBehaviour.goGet(val)    }
  static goRemove(val) { MonoBehaviour.goRemove(val) }

  hasInput             { _has_input             }
  hasInput=(has_input) { _has_input = has_input }

  initializeVariables() {
    _jump_height      = 1.1
    _held_jump        = false
    _held_attack      = false
    _speed_base       = 20.0
    _speed_sprint     = 80.0
    _max_speed        = 17.5
    _max_speed_sprint = 40.0
    _on_the_ground    = true
    _sensitivity      = Vec2.new(300.0, 200.0)

    _rotation         = Vec3.new(0.0)
    _temp_velocity    = Vec3.new(0.0)
    _shurikens        = []
    _time             = 0.0
    _has_input        = true

    _cObject          = null
    _cHolding         = false
    _useNormalMapping = 0.0
  }

  initialize() {
    initializeVariables()
    _entity_camera = GameObject.new()
    _camera_transform = _entity_camera.getComponent(Transform)

    _camera = _entity_camera.addComponent(Camera)
    _camera_transform.localPosition = Vec3.new(0.0, 0.75, 0.0)
    _camera_transform.parent = gameObject
    var listener = _entity_camera.addComponent(WaveSource)
    listener.makeMainListener()
    _camera.far = 250.0

    var collider = gameObject.addComponent(Collider)
    _rigid_body = gameObject.addComponent(RigidBody)
    _rigid_body.angularConstraints = PhysicsConstraints.X | PhysicsConstraints.Z
    transform.localPosition = Vec3.new(0.0, 200.0, 0.0)
    collider.makeCapsuleCollider()
    collider.layers = PhysicsLayers.General

    //var output_pre_z = [ "depth_buffer" ]
    var output_opaque = [ "albedo", "position", "normal", "metallic_roughness", "emissiveness", "depth_buffer" ]
    //var deferred_shader_pre_z = Shader.load("resources/shaders/pre_z_opaque.fx")
    var deferred_shader_opaque = Shader.load("resources/shaders/default_opaque.fx")
    //_camera.addShaderPass("deferred_shader_pre_z", deferred_shader_pre_z, [], output_pre_z)
    _camera.addShaderPass("deferred_shader_opaque", deferred_shader_opaque, [], output_opaque)

    gameObject.name = "player"


    // Add a spot light.
    var light = _entity_camera.addComponent(Light)
    light.type = LightTypes.Point

    light.lightColour = Vec3.new(1.0, 1.0, 1.0)
    light.lightIntensity = 1.0
    light.depth = 2.0
  }

  rotation { _rotation }
  cameraTransform { _camera_transform }
  camera { _camera }

  getHolding            { _has_input ? InputController.MovementUpDown : 0.0 }
  getHeldAttack         { (InputController.CameraAttack   > 0.0 && _has_input) ? true : false }
  getCameraVertical     { _has_input ? InputController.CameraVertical   * (Time.fixedDeltaTime / Time.timeScale) * _sensitivity.x : 0.0 }
  getCameraHorizontal   { _has_input ? InputController.CameraHorizontal * (Time.fixedDeltaTime / Time.timeScale) * _sensitivity.y : 0.0 }
  getMovementVertical   { _has_input ? InputController.MovementVertical   : 0.0 }
  getMovementHorizontal { _has_input ? InputController.MovementHorizontal : 0.0 }
  getMovementSprint     { _has_input ? Math.lerp(_max_speed,  _max_speed_sprint, InputController.MovementSprint) : 0.0 }
  getMovementSprintSpd  { _has_input ? Math.lerp(_speed_base, _speed_sprint,     InputController.MovementSprint) : 0.0 }

  drawDebug() {
    var from = _camera_transform.worldPosition
    var to = from + _camera_transform.worldForward * 2
    Debug.drawLine(to, to + Vec3.new(0.1, 0.0, 0.0), Vec4.new(1.0, 0.0, 0.0, 1.0))
    Debug.drawLine(to, to + Vec3.new(0.0, 0.1, 0.0), Vec4.new(0.0, 1.0, 0.0, 1.0))
    Debug.drawLine(to, to + Vec3.new(0.0, 0.0, 0.1), Vec4.new(0.0, 0.0, 1.0, 1.0))
  }

  handleDynamic() {
    // Select and deselect the dynamic objects.
    var last_holding = _cHolding
    _cHolding = getHolding
    if (_cHolding != 0 && last_holding == 0) {
      if (!_cObject && _cHolding > 0) {
        var from = _camera_transform.worldPosition
        var to = from + _camera_transform.worldForward * 10
        var sorted = Sort.sort(Physics.castRay(from, to), Sorter.new(from))
        
        for (v in sorted) {
          // You cannot select  ground.
          if (v.gameObject.name == "ground") {
            _cObject = null
            break
          }
          // Select the dynamic object.
          if (v.gameObject.name == "cube" || v.gameObject.name == "spear") {
            _cObject = v.gameObject
            break
          }
        }
      } else {
        // Release the dynamic object.
        if (_cHolding > 0.0) {
          var pVel = _camera_transform.worldForward * 20
          _cObject.getComponent(RigidBody).velocity = pVel
        }
        _cObject = null
      }
    }

    if (_cObject) {
      var pFrom = _camera_transform.worldPosition + _camera_transform.worldForward * 5

      var speed = 75
      var rotSpeed = 10
      var diff = pFrom - _cObject.transform.worldPosition
      var len = Math.min(diff.length, speed)
      if (len > 0) {
        len = Math.sqrt(len / speed) * speed
        diff = diff.normalized * len
      }

      //_cObject.getComponent(RigidBody).velocity = diff
      _cObject.getComponent(RigidBody).applyImpulse(diff - _cObject.getComponent(RigidBody).velocity)

      var camForward = _camera_transform.worldForward
      var objForward = _cObject.transform.worldForward

      if (_cObject.name == "cube") {
        _cObject.getComponent(RigidBody).angularVelocity = _cObject.getComponent(RigidBody).angularVelocity * 0.75
      }
    }
  }

  update() {
    drawDebug()
  }

  fixedUpdate() {
    // Respawning.
    if (transform.worldPosition.y < -10) {
      transform.worldPosition = Vec3.new(0.0, 2.0, -0.0) * 2
      gameObject.getComponent(RigidBody).velocity = Vec3.new(0.0)
    }

    handleDynamic()

    var last_held_attack = _held_attack
    _held_attack = getHeldAttack
    if (_held_attack && !last_held_attack) {
      //_shurikens.add(ShurikenBehaviour.create(_camera_transform.worldPosition, _camera_transform.worldForward))
    }

    // Get all data required to move the player / rotate the camera.
    _temp_rotation = _rotation
    _temp_velocity = gameObject.getComponent(RigidBody).velocity

    // Free the player if its stuck.
    //if (InputController.FreeMe > 0.0) {
    //    transform.worldPosition = transform.worldPosition + Vec3.new(0.0, 5.0, 0.0)
    //}

    // Get the new rotation and movement based on the data provided.
    rotateCamera()
    moveCamera()
    modifyVelocity()

    // Apply the new rotation and velocity.
    _rotation = _temp_rotation
    _camera_transform.localEuler = Vec3.new(_rotation.x, 0.0, 0.0) * Math.deg2Rad
    transform.localEuler = Vec3.new(0.0, _rotation.y, 0.0) * Math.deg2Rad
    gameObject.getComponent(RigidBody).velocity = _temp_velocity
  }

  rotateCamera() {
    // Rotate the camera.
    _temp_rotation = _temp_rotation + Vec3.new(getCameraVertical, getCameraHorizontal, 0.0)
    _temp_rotation.x = Math.clamp(_temp_rotation.x, -90.0, 90.0)
    _temp_rotation.y = Math.wrap( _temp_rotation.y, 0.0, 360.0)
  }

  moveCamera() {
    // Do not allow the player to move if we're not on the ground.
    if (!_on_the_ground) return

    // Jumping.
    // var last_held_jump = _held_jump
    // _held_jump = InputController.MovementUpDown > 0.0 ? true : false
    // if (_held_jump && !last_held_jump) {
    //   _temp_velocity.y = Math.sqrt(2.0 * 9.81 * _jump_height)
    // }

    // Find out how badly we need to move.
    var sin = -Math.sin(Math.deg2Rad * _temp_rotation.y)
    var cos =  Math.cos(Math.deg2Rad * _temp_rotation.y)

    var movement = Vec2.new(sin, -cos).normalized * getMovementVertical + Vec2.new(-cos, -sin).normalized * -getMovementHorizontal

    // Move the player.
    var movement_length = movement.lengthSqr
    if (movement_length > 0.0) {
      if (movement_length > 1.0) {
        movement.normalize()
      }

      movement = movement * Time.fixedDeltaTime * getMovementSprintSpd

      var vel_length = Vec2.new(_temp_velocity.x, _temp_velocity.z).length
      var mov_length = movement.length
      var mul = Math.clamp(0.0, 1.0, (getMovementSprint - vel_length) / mov_length)
      _temp_velocity = _temp_velocity + Vec3.new(movement.x, 0.0, movement.y) * mul
    }
  }

  modifyVelocity() {
    // If we're on the ground then we should slow down immensly quickly.
    if (_on_the_ground) {
      _temp_velocity.x = _temp_velocity.x * (1.0 - Time.fixedDeltaTime * 2.0)
      _temp_velocity.z = _temp_velocity.z * (1.0 - Time.fixedDeltaTime * 2.0)
    }

    // Add an extra umpfh to the fall of the jump.
    if (_temp_velocity.y < 0.0) {
      _temp_velocity.y = _temp_velocity.y * (1.0 + Time.fixedDeltaTime)
    }

    var len = _temp_velocity.x * _temp_velocity.x + _temp_velocity.z * _temp_velocity.z
    var max_speed = getMovementSprint
    if (len > max_speed) {
      _temp_velocity.x = (_temp_velocity.x / len) * max_speed
      _temp_velocity.z = (_temp_velocity.z / len) * max_speed
    }
  }
}
