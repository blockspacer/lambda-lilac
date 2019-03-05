import "Core/Vec2" for Vec2
import "Core/Vec3" for Vec3

import "Core/Texture" for Texture
import "Core/Shader" for Shader
import "Core/Mesh" for Mesh
import "Core/Console" for Console

import "Core/GameObject" for GameObject

import "Core/Transform" for Transform
import "Core/Camera" for Camera
import "Core/Lod" for Lod
import "Core/RigidBody" for RigidBody
import "Core/WaveSource" for WaveSource
import "Core/Collider" for Collider
import "Core/MonoBehaviour" for MonoBehaviour
import "Core/Physics" for Physics
import "Core/Manifold" for Manifold

import "Core/Input" for Input, Keys, Buttons, Axes
import "Core/Math" for Math

import "Core/Time" for Time

import "resources/scripts/wren/input_controller" for InputController
import "resources/scripts/wren/shuriken" for ShurikenBehaviour

class FreeLookCamera is MonoBehaviour {
  construct new() { super() }
  static goGet(val) { MonoBehaviour.goGet(val) }
  static goRemove(val) { MonoBehaviour.goRemove(val) }

  hasInput { _has_input }
  hasInput=(has_input) { _has_input = has_input }

  initializeVariables() {
    _jump_height      = 1.1
    _held_jump        = false
    _held_attack      = false
    _speed_base       = 20.0
    _speed_sprint     = 50.0
    _max_speed        = 17.5
    _max_speed_sprint = 20.0
    _on_the_ground    = true
    _sensitivity      = Vec2.new(300.0, 200.0)

    _rotation         = Vec3.new(0.0)
    _temp_velocity    = Vec3.new(0.0)
    _shurikens        = []
    _time             = 0.0
    _attacker         = null
    _attack           = false
    _has_input        = true
  }

  initialize() {
    initializeVariables()
    _entity_camera = GameObject.new()
    _camera_transform = _entity_camera.getComponent(Transform)

    _camera = _entity_camera.addComponent(Camera)
    _camera_transform.localPosition = Vec3.new(0.0, 0.75, 0.0)
    _camera_transform.parent = transform
    var listener = _entity_camera.addComponent(WaveSource)
    listener.makeMainListener()
    _camera.far = 250.0

    var collider = gameObject.addComponent(Collider)
    _rigid_body = gameObject.addComponent(RigidBody)
    _rigid_body.angularConstraints = 1 << 0 + 1 << 2
    transform.localPosition = Vec3.new(0.0, 20.0, 0.0)
    collider.makeCapsuleCollider()

    var output_opaque = [ "albedo", "position", "normal", "metallic_roughness", "depth_buffer" ]
    var deferred_shader_opaque = Shader.load("resources/shaders/default_opaque.fx")
    _camera.addShaderPass("deferred_shader_opaque", deferred_shader_opaque, [], output_opaque)

    gameObject.tags = [ "player" ]
  }

  rotation { _rotation }
  cameraTransform { _camera_transform }

  getAttack             { (InputController.MovementUpDown > 0.0 && _has_input) ? true : false }
  getHeldAttack         { (InputController.CameraAttack   > 0.0 && _has_input) ? true : false }
  getCameraVertical     { _has_input ? InputController.CameraVertical   * Time.fixedDeltaTime * _sensitivity.x : 0.0 }
  getCameraHorizontal   { _has_input ? InputController.CameraHorizontal * Time.fixedDeltaTime * _sensitivity.y : 0.0 }
  getMovementVertical   { _has_input ? InputController.MovementVertical   : 0.0 }
  getMovementHorizontal { _has_input ? InputController.MovementHorizontal : 0.0 }
  getMovementSprint     { _has_input ? Math.lerp(InputController.MovementSprint, _max_speed, _max_speed_sprint) : 0.0 }

  fixedUpdate() {
    // Jumping.
    var last_attack = _attack
    _attack = getAttack
    if (_attack && !last_attack) {
      if (!_attacker) {
        var from = _camera_transform.worldPosition
        var to = from + _camera_transform.worldForward * 10
        for(v in Physics.castRay(from, to)) {
          if (v.gameObject.name == "cube") {
            _attacker = v.gameObject
            break
          }
        }
      } else {
        var pVel = _camera_transform.worldForward * 20
       _attacker.getComponent(RigidBody).velocity = pVel
        _attacker = null
      }
    }

    if (_attacker) {
      var dist = 5
      var pFrom = _camera_transform.worldPosition + _camera_transform.worldForward * dist

      var from = _camera_transform.worldPosition
      var to = from + _camera_transform.worldForward * 5
      for(v in Physics.castRay(from, to)) {
        if (v.gameObject.name == "ground") {
          var l = (_camera_transform.worldPosition - (v.point + v.normal)).length

          if (l < dist) {
            dist = l
            pFrom = v.point + v.normal
          }
        }
      }

      var pEuler = Vec2.new(_camera_transform.worldForward.x, _camera_transform.worldForward.z)
      pEuler.normalize()
      pEuler = Vec3.new(0.0, Math.atan2(pEuler.x, pEuler.y), 0.0)
      
      _attacker.transform.worldPosition = pFrom
      _attacker.transform.worldEuler = pEuler
      _attacker.getComponent(RigidBody).velocity = Vec3.new(0.0)
      _attacker.getComponent(RigidBody).angularVelocity = Vec3.new(0.0)
    }


    var last_held_attack = _held_attack
    _held_attack = getHeldAttack
    if (_held_attack && !last_held_attack) {
      _shurikens.add(ShurikenBehaviour.create(_camera_transform.worldPosition, _camera_transform.worldForward))
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

      //var speed = _speed_base + InputController.MovementSprint * (_speed_sprint - _speed_base)
      var speed = _speed_base
      movement = movement * Time.fixedDeltaTime * speed

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
