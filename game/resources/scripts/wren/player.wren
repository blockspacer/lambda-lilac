import "Core/Vec2"               for Vec2
import "Core/Vec3"               for Vec3
import "Core/Vec4"               for Vec4
import "Core/Texture"            for Texture
import "Core/Shader"             for Shader
import "Core/Mesh"               for Mesh
import "Core/Console"            for Console
import "Core/GameObject"         for GameObject
import "Core/Transform"          for Transform
import "Core/Camera"             for Camera
import "Core/Lod"                for Lod
import "Core/RigidBody"          for RigidBody
import "Core/PhysicsConstraints" for PhysicsConstraints
import "Core/WaveSource"         for WaveSource
import "Core/Collider"           for Collider
import "Core/MonoBehaviour"      for MonoBehaviour
import "Core/Physics"            for Physics
import "Core/Manifold"           for Manifold
import "Core/Input"              for Input
import "Core/Input"              for Keys
import "Core/Input"              for Buttons
import "Core/Input"              for Axes
import "Core/Math"               for Math
import "Core/Time"               for Time
import "Core/Debug"              for Debug
import "Core/Sort"               for Sort
import "Core/PostProcess"        for PostProcess
import "Core/Light"              for Light
import "Core/Light"              for LightTypes
import "Core/Light"              for ShadowTypes
import "Core/MeshRender"         for MeshRender

import "resources/scripts/wren/input_controller" for InputController
import "resources/scripts/wren/physics_layers"   for PhysicsLayers
import "resources/scripts/wren/camera"           for Sorter

class ThirdPersonCamera is MonoBehaviour {
  construct new()      { super()                     }
  static goGet(val)    { MonoBehaviour.goGet(val)    }
  static goRemove(val) { MonoBehaviour.goRemove(val) }

  hasInput            { _hasInput            }
  hasInput=(hasInput) { _hasInput = hasInput }

  initializeVariables() {
    _speedWalk       = 20.0
    _speedSprint     = 80.0
    _speedWall       = 20.0
    
    _maxSpeedWalk    = 17.5
    _maxSpeedSprint  = 40.0
    
    _sensitivity     = Vec2.new(300.0, 200.0)
    _jumpHeight      = 5.0

    _rotation        = Vec3.new(0.0)
    _velocity        = Vec3.new(0.0)
    _hasInput        = true
    _onTheGround     = true
  }

  initialize() {
    gameObject.name         = "player"
    transform.localPosition = Vec3.new(0.0, 2.0, 0.0)

    // Add a cube to visualize.
    var meshRender     = gameObject.addComponent(MeshRender)
    meshRender.mesh    = Mesh.generate("cube")
    meshRender.subMesh = 0

    // Initialize variables.
    initializeVariables()

    // Create the in-between node.
    _entityInBetween           = GameObject.new()
    _transformInBetween        = _entityInBetween.transform
    _transformInBetween.parent = gameObject

    // Create the camera.
    _entityCamera                  = GameObject.new()
    _transformCamera               = _entityCamera.transform
    _transformCamera.localPosition = Vec3.new(0.0, 1.0, 5.0)
    _transformCamera.parent        = _entityInBetween
    
    // Add the camera.
    _camera     = _entityCamera.addComponent(Camera)
    _camera.far = 250.0
    
    // Add the listener.
    var listener = _entityCamera.addComponent(WaveSource)
    listener.makeMainListener()

    // Add the collider.
    var collider    = gameObject.addComponent(Collider)
    collider.layers = PhysicsLayers.General
    collider.makeCapsuleCollider()

    // Add the rigid body.
    _rigidBody                    = gameObject.addComponent(RigidBody)
    _rigidBody.angularConstraints = PhysicsConstraints.X | PhysicsConstraints.Y | PhysicsConstraints.Z
    
    // Add the render passes.
    /*var outputPreZ = [ "depth_buffer" ]
    var deferredShaderPreZ = Shader.load("resources/shaders/pre_z_opaque.fx")
    _camera.addShaderPass("deferredShaderPreZ", deferredShaderPreZ, [], outputPreZ)*/
    var outputOpaque         = [ "albedo", "position", "normal", "metallic_roughness", "emissiveness", "depth_buffer" ]
    var deferredShaderOpaque = Shader.load("resources/shaders/default_opaque.fx")
    _camera.addShaderPass("deferredShaderOpaque", deferredShaderOpaque, [], outputOpaque)
    _fovDefault = 90.0
    _fovWall = 80.0
    _fov = _fovDefault

    // Add a point light.
    var light            = _entityCamera.addComponent(Light)
    light.type           = LightTypes.Point
    light.lightColour    = Vec3.new(1.0, 1.0, 1.0)
    light.lightIntensity = 1.0
    light.depth          = 2.0
  }

  rotation { _rotation }
  cameraTransform { _transformCamera }
  camera { _camera }

  cameraVertical     { _hasInput ? InputController.CameraVertical   * (Time.fixedDeltaTime / Time.timeScale) * _sensitivity.x : 0.0 }
  cameraHorizontal   { _hasInput ? InputController.CameraHorizontal * (Time.fixedDeltaTime / Time.timeScale) * _sensitivity.y : 0.0 }
  movementVertical   { _hasInput ? InputController.MovementVertical   : 0.0 }
  movementHorizontal { _hasInput ? InputController.MovementHorizontal : 0.0 }
  movementSprint     { _hasInput ? (_onTheGround ? Math.lerp(_maxSpeedWalk, _maxSpeedSprint, InputController.MovementSprint) : _maxSpeedWalk) : 0.0 }
  movementSprintSpd  { _hasInput ? (_onTheGround ? Math.lerp(_speedWalk,    _speedSprint,    InputController.MovementSprint) : _speedWalk)    : 0.0 }

  fixedUpdate() {
    // Respawning.
    if (transform.worldPosition.y < -10) {
      transform.worldPosition = Vec3.new(0.0, 2.0, -0.0) * 2
      _rigidBody.velocity = Vec3.new(0.0)
    }

    // Get all data required to move the player / rotate the camera.
    _velocity = _rigidBody.velocity

    // Get the new rotation and movement based on the data provided.
    rotateCamera()
    moveCamera()
    modifyVelocity()

    // Apply the new rotation and velocity.
    if (_onWall) {
      var diff = 65.0 - _rotation.x

      var onWallRotation = _onWallRotation - 90

      // Something needs to be done.
      var delta = onWallRotation - _rotation.y 
      if (Math.abs(delta + 360) < Math.abs(delta)) {
        onWallRotation = onWallRotation + 360
      }
      if (Math.abs(delta - 360) < Math.abs(delta)) {
        onWallRotation = onWallRotation - 360
      }

      _rotation.x = Math.lerp(_rotation.x, _rotation.x + diff, 0.1)
      _rotation.y = Math.lerp(_rotation.y, onWallRotation, 0.1)

      _fov = Math.lerp(_fov, _fovWall, 0.1)

      _transformInBetween.localEuler = Vec3.new(_rotation.x, _rotation.y - _onWallRotation, 0.0) * Math.deg2Rad
      transform.localEuler           = Vec3.new(0.0, _onWallRotation * Math.deg2Rad, 0.0)
    } else {
      _fov = Math.lerp(_fov, _fovDefault, 0.1)
      transform.localEuler           = Vec3.new(0.0, _rotation.y, 0.0) * Math.deg2Rad
      _transformInBetween.localEuler = Vec3.new(_rotation.x, 0.0, 0.0) * Math.deg2Rad
    }
    _rigidBody.velocity = _velocity
    _camera.fovDeg = _fov
  }

  rotateCamera() {
    // Rotate the camera.
    _rotation = _rotation + Vec3.new(cameraVertical, cameraHorizontal, 0.0)
    _rotation.x = Math.clamp(_rotation.x, -90.0, 90.0)
    _rotation.y = Math.wrap( _rotation.y, 0.0, 360.0)
  }

  dirToDeg(dir) {
    //if (dir.x == 1) return 0
    //if (dir.z == 1) return 90
    //if (dir.x == -1) return 180
    //if (dir.z == -1) return 270
    return Math.atan2(-dir.z, dir.x) * Math.rad2Deg
  }

  wallRun(dir) {
    var hits = Physics.castRay(transform.worldPosition, transform.worldPosition + dir)
    if (hits.count > 0) {
      var hit = Sort.sort(hits, Sorter.new(transform.worldPosition))[0]
      transform.worldPosition = hit.point - dir * 0.75
      _velocity = Vec3.new(0, _speedWall, 0)
      _onWall  = true
      _onWallDir = dir
      _onWallRotation = dirToDeg(dir)
      if (_onWallRotation < 0) _onWallRotation = _onWallRotation + 360
      return true
    }
    return false
  }

  checkOnGround() {
    _onTheGround = false
    var hits = Physics.castRay(transform.worldPosition, transform.worldPosition + Vec3.new(0.0, -0.751, 0.0))
    if (hits.count > 0) {
      _onTheGround = true
    }
  }

  moveCamera() {
    checkOnGround()

    // Wall run.
    var wasOnWall = _onWall
    if (_onWall) {
      _onWall = wallRun(_onWallDir)
    } else {
      if (InputController.MovementUpDown > 0.0) {
        if (!_onWall && wallRun(Vec3.new( 1.0, 0.0, 0.0))) _onWall = true
        if (!_onWall && wallRun(Vec3.new(-1.0, 0.0, 0.0))) _onWall = true
        if (!_onWall && wallRun(Vec3.new(0.0, 0.0,  1.0))) _onWall = true
        if (!_onWall && wallRun(Vec3.new(0.0, 0.0, -1.0))) _onWall = true
        _onWallLastPressed = true
      }
    }

    // Jumping.
    var lastHeldJump = _heldJump
    _heldJump = (InputController.MovementUpDown > 0.0 ? true : false)
    if (_heldJump && !lastHeldJump && _onTheGround) {
      _velocity.y = Math.sqrt(2.0 * -Physics.gravity.y * _jumpHeight)
    }
    if (_heldJump && !lastHeldJump && _onWall) {
      _velocity = Vec3.new(-_onWallDir.x, 1.0, -_onWallDir.z) * _speedWall
      wasOnWall = false
    }

    if (wasOnWall && !_onWall) {
      _velocity.y = Math.min(_velocity.y, _jumpHeight)
    }

    // Find out how badly we need to move.
    var sin = -Math.sin(Math.deg2Rad * _rotation.y)
    var cos =  Math.cos(Math.deg2Rad * _rotation.y)

    var movement = Vec2.new(sin, -cos).normalized * movementVertical + Vec2.new(-cos, -sin).normalized * -movementHorizontal

    // Move the player.
    var movementLength = movement.lengthSqr
    if (movementLength > 0.0) {
      if (movementLength > 1.0) {
        movement.normalize()
      }

      movement = movement * Time.fixedDeltaTime * movementSprintSpd

      var lengthVel = Vec2.new(_velocity.x, _velocity.z).length
      var lengthMov = movement.length
      var mul = Math.clamp(0.0, 1.0, (movementSprint - lengthVel) / lengthMov)
      _velocity = _velocity + Vec3.new(movement.x, 0.0, movement.y) * mul
    }
  }

  modifyVelocity() {
    // If we're on the ground then we should slow down immensly quickly.
    if (_onTheGround) {
      _velocity.x = _velocity.x * (1.0 - Time.fixedDeltaTime * 5.0)
      _velocity.z = _velocity.z * (1.0 - Time.fixedDeltaTime * 5.0)
    }

    // Add an extra umpfh to the fall of the jump.
    if (_velocity.y < 0.0) {
      _velocity.y = Math.clamp(_velocity.y * (1.0 + Time.fixedDeltaTime * 3.0), Physics.gravity.y * 3.0, 0.0)
    } else {
      if (_velocity.y < 2.0) {
        _velocity.y = _velocity.y - Time.fixedDeltaTime * 2.0
      }
    }

    var maxSpeed = movementSprint
    var len = Vec2.new(_velocity.x, _velocity.z).length
    if (len > maxSpeed) {
      _velocity.x = (_velocity.x / len) * maxSpeed
      _velocity.z = (_velocity.z / len) * maxSpeed
    }
  }
}
