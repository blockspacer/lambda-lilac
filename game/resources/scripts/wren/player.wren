import "Core" for Vec2, Vec3, Vec4, Quat
import "Core" for Texture, Shader, Mesh
import "Core" for GameObject, Transform, Camera, Lod, RigidBody, WaveSource, Collider, MonoBehaviour, Light, MeshRender
import "Core" for PhysicsConstraints, Physics, Manifold
import "Core" for Input, Keys, Buttons, Axes
import "Core" for Math, Time, Debug, Sort, Console
import "Core" for PostProcess, LightTypes, ShadowTypes

import "resources/scripts/wren/input_controller" for InputController
import "resources/scripts/wren/physics_layers"   for PhysicsLayers
import "resources/scripts/wren/camera"           for Sorter
import "resources/scripts/wren/post_processor"   for PostProcessor
import "resources/scripts/wren/meshes"           for Meshes

class Arm {
  construct new(parent) {
    _shoulder = GameObject.new()
    _shoulder.transform.parent        = parent
    _shoulder.transform.localPosition = Vec3.new(0.0, 0.0, -0.5)
    
    _upperArm = GameObject.new()
    _upperArm.transform.parent = _shoulder
    var meshRender     = _upperArm.addComponent(MeshRender)
    meshRender.mesh    = Meshes.cube
    meshRender.subMesh = 0
    _upperArm.transform.localScale    = Vec3.new(0.25, 0.25, 1.0)
    _upperArm.transform.localPosition = Vec3.new(0.0, 0.0, -0.5)
    
    _elbow = GameObject.new()
    _elbow.transform.parent        = _shoulder
    _elbow.transform.localPosition = Vec3.new(0.0, 0.0, -1.0)
    
    _lowerArm = GameObject.new()
    _lowerArm.transform.parent = _elbow
    meshRender         = _lowerArm.addComponent(MeshRender)
    meshRender.mesh    = Meshes.cube
    meshRender.subMesh = 0
    _lowerArm.transform.localScale    = Vec3.new(0.25, 0.25, 1.0)
    _lowerArm.transform.localPosition = Vec3.new(0.0, 0.0, -0.5)
  }
  shoulder { _shoulder }
  elbow    { _elbow }
}

class Hooman {
  construct new(parent) {
    var mesh = Meshes.cube
    var core = GameObject.new()
    core.transform.parent = parent
    
    var body = GameObject.new()
    body.transform.parent = core
    var meshRender     = body.addComponent(MeshRender)
    meshRender.mesh    = mesh
    meshRender.subMesh = 0
    body.transform.localScale = Vec3.new(1.0, 2.0, 0.5)
    body.transform.localPosition = Vec3.new(0.0, 0.5, 0.0)
    
    _neck = GameObject.new()
    _neck.transform.parent = core
    _neck.transform.localPosition = Vec3.new(0.0, 1.5, 0.0)
    
    var head = GameObject.new()
    head.transform.parent = _neck
    meshRender         = head.addComponent(MeshRender)
    meshRender.mesh    = mesh
    meshRender.subMesh = 0
    head.transform.localScale = Vec3.new(0.75)
    head.transform.localPosition = Vec3.new(0.0, 0.5, 0.0)

    _arm1 = Arm.new(core)
    _arm1.shoulder.transform.localPosition = Vec3.new( -0.5,  1.5, 0.0)
    _arm2 = Arm.new(core)
    _arm2.shoulder.transform.localPosition = Vec3.new(  0.5,  1.5, 0.0)
    _leg1 = Arm.new(core)
    _leg1.shoulder.transform.localPosition = Vec3.new(-0.25, -0.5, 0.0)
    _leg2 = Arm.new(core)
    _leg2.shoulder.transform.localPosition = Vec3.new( 0.25, -0.5, 0.0)
    
    _currShoulder1Euler = Vec3.new(0.0)
    _currShoulder2Euler = Vec3.new(0.0)
    _currElbow1Euler = Vec3.new(0.0)
    _currElbow2Euler = Vec3.new(0.0)
    _currThigh1Euler = Vec3.new(0.0)
    _currThigh2Euler = Vec3.new(0.0)
    _currKnee1Euler = Vec3.new(0.0)
    _currKnee2Euler = Vec3.new(0.0)
    _currNeckEuler = Vec3.new(0.0)
    
    _shoulder1Euler = Vec3.new(0.0)
    _shoulder2Euler = Vec3.new(0.0)
    _elbow1Euler = Vec3.new(0.0)
    _elbow2Euler = Vec3.new(0.0)
    _thigh1Euler = Vec3.new(0.0)
    _thigh2Euler = Vec3.new(0.0)
    _knee1Euler = Vec3.new(0.0)
    _knee2Euler = Vec3.new(0.0)
    _neckEuler = Vec3.new(0.0)

    _walkingSpeed = 0.0
    _minWalkingSpeed = 0.25
    _maxWalkingSpeed = 1.0

    _totalTime = 0.0
  }
  
  keepItTogether(a, b) {
      var delta = b.x - a.x
      if (Math.abs(delta + Math.tau) < Math.abs(delta)) b.x = b.x + Math.tau
      if (Math.abs(delta - Math.tau) < Math.abs(delta)) b.x = b.x - Math.tau
      delta = b.y - a.y 
      if (Math.abs(delta + Math.tau) < Math.abs(delta)) b.y = b.y + Math.tau
      if (Math.abs(delta - Math.tau) < Math.abs(delta)) b.y = b.y - Math.tau
      return b
  }

  vecLerp(a, b, v) { 
    return Vec3.new(Math.lerp(a.x, b.x, v), Math.lerp(a.y, b.y, v), Math.lerp(a.z, b.z, v)) 
  }

  rotLerp(a, b) {
    return vecLerp(a, keepItTogether(a, b), 0.1)    
  }

  fixedUpdate() {
    _totalTime = _totalTime + Time.fixedDeltaTime
    
    if (inAir || onWall) {
      if (onWall) {
        animWall()
      } else {
        animFall()
      }
    } else {
      if (_walkingSpeed <= _minWalkingSpeed) {
        animIdle()
      } else {
        animWalk()
      }
    }

    _currShoulder1Euler = rotLerp(_currShoulder1Euler, _shoulder1Euler)
    _currShoulder2Euler = rotLerp(_currShoulder2Euler, _shoulder2Euler)
    _currElbow1Euler = rotLerp(_currElbow1Euler, _elbow1Euler)
    _currElbow2Euler = rotLerp(_currElbow2Euler, _elbow2Euler)
    _currThigh1Euler = rotLerp(_currThigh1Euler, _thigh1Euler)
    _currThigh2Euler = rotLerp(_currThigh2Euler, _thigh2Euler)
    _currKnee1Euler = rotLerp(_currKnee1Euler, _knee1Euler)
    _currKnee2Euler = rotLerp(_currKnee2Euler, _knee2Euler)
    _currNeckEuler = rotLerp(_currNeckEuler, _neckEuler)
    
    _arm1.shoulder.transform.localEuler = _currShoulder1Euler
    _arm1.elbow.transform.localEuler    = _currElbow1Euler
    _arm2.shoulder.transform.localEuler = _currShoulder2Euler
    _arm2.elbow.transform.localEuler    = _currElbow2Euler
    
    _leg1.shoulder.transform.localEuler = _currThigh1Euler
    _leg1.elbow.transform.localEuler    = _currKnee1Euler
    _leg2.shoulder.transform.localEuler = _currThigh2Euler
    _leg2.elbow.transform.localEuler    = _currKnee2Euler

    _neck.transform.localEuler = _currNeckEuler
  }

  walkingSpeed=(walkingSpeed) { _walkingSpeed = walkingSpeed }
  walkingSpeed { _walkingSpeed }
  minWalkingSpeed=(minWalkingSpeed) { _minWalkingSpeed = minWalkingSpeed }
  minWalkingSpeed { _minWalkingSpeed }
  maxWalkingSpeed=(maxWalkingSpeed) { _maxWalkingSpeed = maxWalkingSpeed }
  maxWalkingSpeed { _maxWalkingSpeed }
  inAir=(inAir) { _inAir = inAir }
  inAir { _inAir }
  onWall=(onWall) { _onWall = onWall }
  onWall { _onWall }

  animIdle() {
    var halfPi = Math.pi / 2.0
    var armWobbleX = Math.sin(_totalTime * 1.0) * 0.125
    var armWobbleY = Math.cos(_totalTime * 0.25) * Math.sin(_totalTime * 6.0) * 0.125
    var shoulderWobbleX = Math.cos(_totalTime * 1.0) * Math.sin(_totalTime * 8.0) * 0.125 + 0.125
    var thighWobbleX = Math.sin(_totalTime * 1.25) * 0.125
    var thighWobbleY = Math.cos(_totalTime * 0.5) * Math.sin(_totalTime * 1.0) * 0.125
    var kneeWobbleX = Math.cos(_totalTime * 1.5) * Math.sin(_totalTime * 0.75) * 0.125 + 0.125

    _shoulder1Euler = Vec3.new(0.0, armWobbleY - halfPi, -halfPi * 1.05 - armWobbleX)
    _elbow1Euler    = Vec3.new(0.0, shoulderWobbleX, 0.0)
    _shoulder2Euler = Vec3.new(0.0, armWobbleY - halfPi, -halfPi * 0.95 + armWobbleX)
    _elbow2Euler    = Vec3.new(0.0, shoulderWobbleX, 0.0)
    
    _thigh1Euler = Vec3.new(0.0, thighWobbleY - halfPi, -halfPi - thighWobbleX)
    _knee1Euler  = Vec3.new(0.0, -kneeWobbleX, 0.0)
    _thigh2Euler = Vec3.new(0.0, thighWobbleY - halfPi, -halfPi + thighWobbleX)
    _knee2Euler  = Vec3.new(0.0, -kneeWobbleX, 0.0)
    
    if (_totalTime % 7.0 >= 5.0) {
      var time = _totalTime % 10.0 - 5.0
      time = time * Math.pi
      _neckEuler = Vec3.new(0.0, Math.sin(time) * 1.0, 0.0)
    } else {
      _neckEuler = Vec3.new(0.0)
    } 
  }
  animWalk() {
    var intensityMultiplier = Math.clamp((_walkingSpeed - _minWalkingSpeed) / (_maxWalkingSpeed - _minWalkingSpeed), 0.0, 1.0)
    var sin = Math.sin(_totalTime * 10.0) * intensityMultiplier
    var cos = Math.cos(_totalTime * 10.0) * intensityMultiplier
    var absSin = Math.abs(sin)
    var absCos = Math.abs(cos)
    var halfPi = Math.pi / 2.0

    _shoulder1Euler = Vec3.new(0.0, sin - halfPi, -halfPi)
    _elbow1Euler    = Vec3.new(0.0, absSin, 0.0)
    _shoulder2Euler = Vec3.new(0.0, cos - halfPi, -halfPi)
    _elbow2Euler    = Vec3.new(0.0, absCos, 0.0)
    
    _thigh1Euler = Vec3.new(0.0, sin - halfPi, -halfPi)
    _knee1Euler  = Vec3.new(0.0, -absSin, 0.0)
    _thigh2Euler = Vec3.new(0.0, cos - halfPi, -halfPi)
    _knee2Euler  = Vec3.new(0.0, -absCos, 0.0)
    
    _neckEuler = Vec3.new(Math.sin(_totalTime * intensityMultiplier * 10.0) * 0.05 + -0.2, Math.cos(_totalTime * intensityMultiplier * 3.0) * 0.1, 0.0)
  }
  animFall() {
    var halfPi = Math.pi / 2.0
    var armWobbleX = Math.sin(_totalTime * 4.0) * 0.25
    var armWobbleY = Math.cos(_totalTime * 3.0) * Math.sin(_totalTime * 6.0) * 0.25
    var shoulderWobbleX = Math.cos(_totalTime * 4.0) * Math.sin(_totalTime * 8.0) * 0.25 + 0.25
    var thighWobbleX = Math.sin(_totalTime * 6.0) * 0.25
    var thighWobbleY = Math.cos(_totalTime * 2.0) * Math.sin(_totalTime * 4.0) * 0.25
    var kneeWobbleX = Math.cos(_totalTime * 7.0) * Math.sin(_totalTime * 3.0) * 0.25 + 0.25

    _shoulder1Euler = Vec3.new(0.0, armWobbleY - halfPi, -halfPi * 2 - armWobbleX)
    _elbow1Euler    = Vec3.new(shoulderWobbleX, 0.0, 0.0)
    _shoulder2Euler = Vec3.new(0.0, armWobbleY - halfPi, -halfPi * 0 + armWobbleX)
    _elbow2Euler    = Vec3.new(-shoulderWobbleX, 0.0, 0.0)
    
    _thigh1Euler = Vec3.new(0.0, thighWobbleY - halfPi, -halfPi * 1.25 - thighWobbleX)
    _knee1Euler  = Vec3.new(0.0, -kneeWobbleX, 0.0)
    _thigh2Euler = Vec3.new(0.0, thighWobbleY - halfPi, -halfPi * 0.75 + thighWobbleX)
    _knee2Euler  = Vec3.new(0.0, -kneeWobbleX, 0.0)
    
    _neckEuler = Vec3.new(0.0)
  }
  animWall() {
    var intensityMultiplier = 1.0
    var sin = Math.abs(Math.sin(_totalTime * 10.0) * intensityMultiplier)
    var cos = Math.abs(Math.cos(_totalTime * 10.0) * intensityMultiplier)
    var absSin = Math.abs(sin)
    var absCos = Math.abs(cos)
    var halfPi = Math.pi / 2.0

    _shoulder1Euler = Vec3.new(0.0, sin - halfPi, -halfPi)
    _elbow1Euler    = Vec3.new(0.0, absSin, 0.0)
    _shoulder2Euler = Vec3.new(0.0, cos - halfPi, -halfPi)
    _elbow2Euler    = Vec3.new(0.0, absCos, 0.0)
    
    _thigh1Euler = Vec3.new(0.0, sin - halfPi, -halfPi)
    _knee1Euler  = Vec3.new(0.0, -absSin, 0.0)
    _thigh2Euler = Vec3.new(0.0, cos - halfPi, -halfPi)
    _knee2Euler  = Vec3.new(0.0, -absCos, 0.0)
    
    _neckEuler = Vec3.new(halfPi / 2 + Math.sin(_totalTime * 2.0) * 0.2, 0.0, 0.0)
  }
}

class ThirdPersonCamera is MonoBehaviour {
  construct new()      { super()                     }
  static goGet(val)    { MonoBehaviour.goGet(val)    }
  static goRemove(val) { MonoBehaviour.goRemove(val) }

  hasInput            { _hasInput            }
  hasInput=(hasInput) { _hasInput = hasInput }

  initializeVariables() {
    _speedWalk       = 60.0
    _speedSprint     = 100.0
    _speedWall       = 20.0
    
    _maxSpeedWalk    = 17.5
    _maxSpeedSprint  = 40.0
    
    _sensitivity     = Vec2.new(90.0, 90.0)
    _jumpHeight      = 5.0

    _playerRotation  = Vec3.new(0.0)
    _velocity        = Vec3.new(0.0)
    _avgVelocity     = Vec3.new(0.0)
    _hasInput        = true
    _onTheGround     = true
    _camPoint        = Vec3.new(2.0)
    _followDistance  = 5.0
    _mod             = Vec3.new(0.0)

    _fovWalk = 90.0
    _fovRun  = 110.0
    _fovWall = 80.0
    _fov     = _fovWalk
    _fovGoto = _fov
  }

  makeMesh() {
  }

  updateMesh() {
    var velLength = Vec2.new(_velocity.x, _velocity.z).length
    _hooman.inAir = !_onTheGround
    _hooman.onWall = _onWall
    _hooman.maxWalkingSpeed = _speedWalk * 0.5
    _hooman.walkingSpeed = velLength

    _hooman.fixedUpdate()
  }

  initialize() {
    gameObject.name         = "player"
    transform.localPosition = Vec3.new(0.0, 2.0, 0.0)

    _hooman = Hooman.new(gameObject)

    makeMesh()

    // Initialize variables.
    initializeVariables()

    // Create in between.
    _entityInBetween           = GameObject.new()
    _transformInBetween        = _entityInBetween.transform
    _transformInBetween.parent = gameObject
    
    // Create the camera.
    _entityCamera                  = GameObject.new()
    _transformCamera               = _entityCamera.transform
    _transformCamera.localPosition = Vec3.new(0.0, 1.0, 0.0)
    _transformCamera.parent        = _entityInBetween
    
    // Add the camera.
    _camera     = _entityCamera.addComponent(Camera)
    _camera.far = 250.0
    
    // Add the listener.
    var listener = _entityCamera.addComponent(WaveSource)
    listener.makeMainListener()

    // Add the collider.
    var entityCollider = GameObject.new()
    entityCollider.name = "playerBody"
    entityCollider.transform.parent = gameObject
    entityCollider.transform.localScale = Vec3.new(1.0, 3.0, 1.0)
    var collider    = entityCollider.addComponent(Collider)
    collider.layers = PhysicsLayers.General
    collider.makeCapsuleCollider()

    // Add the rigid body.
    _rigidBody                    = entityCollider.addComponent(RigidBody)
    _rigidBody.angularConstraints = PhysicsConstraints.X | PhysicsConstraints.Y | PhysicsConstraints.Z
    
    // Add the render passes.
    /*var outputPreZ = [ "depth_buffer" ]
    var deferredShaderPreZ = Shader.load("resources/shaders/pre_z_opaque.fx")
    _camera.addShaderPass("deferredShaderPreZ", deferredShaderPreZ, [], outputPreZ)*/
    var checker = PostProcessor.getUseChecker ? "|CHECKER" : ""
    var outputOpaque         = [ "albedo", "position", "normal", "metallic_roughness", "emissiveness", "depth_buffer" ]
    var deferredShaderOpaque = Shader.load("resources/shaders/default_opaque.fx%(checker)")
    _camera.addShaderPass("deferredShaderOpaque", deferredShaderOpaque, [], outputOpaque)

    // Add a point light.
    var light            = _entityCamera.addComponent(Light)
    light.type           = LightTypes.Point
    light.lightColour    = Vec3.new(1.0, 1.0, 1.0)
    light.lightIntensity = 1.0
    light.depth          = 2.0
  }

  rotation { _playerRotation }
  cameraTransform { _transformCamera }
  camera { _camera }

  cameraVertical     { _hasInput ? InputController.CameraVertical   * (Time.fixedDeltaTime / Time.timeScale) * _sensitivity.x : 0.0 }
  cameraHorizontal   { _hasInput ? InputController.CameraHorizontal * (Time.fixedDeltaTime / Time.timeScale) * _sensitivity.y : 0.0 }
  movementVertical   { _hasInput ? InputController.MovementVertical   : 0.0 }
  movementHorizontal { _hasInput ? InputController.MovementHorizontal : 0.0 }
  movementSprint     { _hasInput ? (_onTheGround ? Math.lerp(_maxSpeedWalk, _maxSpeedSprint, InputController.MovementSprint) : _maxSpeedWalk) : 0.0 }
  movementSprintSpd  { _hasInput ? (_onTheGround ? Math.lerp(_speedWalk,    _speedSprint,    InputController.MovementSprint) : _speedWalk)    : 0.0 }

  keepIt360(rot) {
      if (rot.x < 0.0) rot.x = rot.x + (-rot.x / 360 + 1).round * 360
      if (rot.y < 0.0) rot.y = rot.y + (-rot.y / 360 + 1).round * 360
      if (rot.x > 360.0) rot.x = Math.wrap(rot.x, 0, 360)
      if (rot.y > 360.0) rot.y = Math.wrap(rot.y, 0, 360)
      return rot
  }
  
  keepItTogether(a, b) {
      var delta = b.x - a.x
      if (Math.abs(delta + 360) < Math.abs(delta)) b.x = b.x + 360
      if (Math.abs(delta - 360) < Math.abs(delta)) b.x = b.x - 360
      delta = b.y - a.y 
      if (Math.abs(delta + 360) < Math.abs(delta)) b.y = b.y + 360
      if (Math.abs(delta - 360) < Math.abs(delta)) b.y = b.y - 360
      return b
  }

  vecLerp(a, b, v) { Vec3.new(Math.lerp(a.x, b.x, v), Math.lerp(a.y, b.y, v), Math.lerp(a.z, b.z, v)) }

  fixedUpdate() {
    updateMesh()
    gameObject.transform.worldPosition = _rigidBody.gameObject.transform.worldPosition - Vec3.new(0.0, -1.0, 0.0)
    _rigidBody.gameObject.transform.localPosition = Vec3.new(0.0, -1.0, 0.0)

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
      _playerRotation = Vec3.new(0.0, _onWallRotation, 0.0)
    }

    transform.localEuler = _playerRotation * Math.deg2Rad

    _rigidBody.velocity = _velocity
    _camera.fovDeg = _fov

    var mod = Vec3.new(0.0, 3.0, 0.0)
    if (_onWall) mod = mod - _onWallDir

    _mod = Math.lerp(_mod, mod, 1.0)

    var delta = (transform.worldPosition + _mod) - _camPoint
    var length = delta.length
    delta.normalize()

    // FOV.
    if (_onWall) _fovGoto = Math.lerp(_fovGoto, _fovWall, Math.clamp(delta.y, 0.0, 1.0))
    _fov = Math.lerp(_fov, _fovGoto, 0.1)

    // Move the camera towards the player.
    _camPoint = vecLerp(_camPoint, _camPoint + delta * (length - _followDistance), 0.1)

    // Rotate the camera towards the player.
    _transformInBetween.worldPosition = _camPoint
    delta = (_camPoint - transform.worldPosition).normalized
    _transformInBetween.worldRotation = Math.lookRotation(delta, Vec3.new(0.0, 1.0, 0.0))
    
    // Keep the camera out of walls.
    var offset = (_transformCamera.worldPosition - transform.worldPosition) * 0.2
    var sorted = Sort.sort(Physics.castRay(transform.worldPosition, _transformCamera.worldPosition + offset * 1.1), Sorter.new(transform.worldPosition))
    if (sorted.count > 0) {
      if (sorted[0].gameObject.name != "playerBody") {
        _transformCamera.worldPosition = sorted[0].point - offset
      }
    } else {
      var localPosition = _transformCamera.localPosition
      localPosition = vecLerp(_transformCamera.localPosition, Vec3.new(0.0, 1.0, 0.0), 0.01)
      _transformCamera.localPosition = localPosition
    }
  }

  rotateCamera() {
    // Rotate the camera.
    _playerRotation.y = _playerRotation.y - movementHorizontal * 180.0 * (Time.fixedDeltaTime / Time.timeScale)

    //if (!_onWall) {
      var delta  = _camPoint - transform.worldPosition
      var length = delta.length
      var depth  = Math.sqrt(1 - (delta.y / length) * (delta.y / length))

      var rotY = Math.atan2(delta.z / length, delta.x / length)
      rotY = rotY - (cameraHorizontal * 120.0 * (Time.fixedDeltaTime / Time.timeScale)) * Math.deg2Rad

      var rotZ = Math.atan2(delta.y / length, depth)
      rotZ = rotZ - (cameraVertical * 120.0 * (Time.fixedDeltaTime / Time.timeScale)) * Math.deg2Rad

      var desiredDelta = Vec3.new(Math.cos(rotY) * depth, Math.sin(rotZ), Math.sin(rotY) * depth).normalized * length
      delta = Math.lerp(delta, desiredDelta, 0.1)

      _camPoint = delta + transform.worldPosition
    //}
  }

  dirToDeg(dir) {
    return Math.atan2(-dir.z, dir.x) * Math.rad2Deg
  }

  wallRun(dir) {
    var offset = Vec3.new(0.0, 1.5, 0.0)
    var from   = transform.worldPosition - offset
    var to     = from + dir
    var hits = Physics.castRay(from, to)
    if (hits.count > 0) {
      var sorted = Sort.sort(hits, Sorter.new(from))
      if (sorted[0].gameObject.name == "playerBody") sorted.removeAt(0)

      if (sorted.count > 0) {
        var hit = sorted[0]
        transform.worldPosition = hit.point - dir * 0.75 + offset
        _velocity = Vec3.new(0, _speedWall, 0)
        _onWall  = true
        _onWallDir = dir
        _onWallRotation = dirToDeg(dir) - 90
        return true
      }
    }
    return false
  }

  checkOnGround() {
    _onTheGround = false
    var hits = Physics.castRay(transform.worldPosition, transform.worldPosition + Vec3.new(0.0, -2.751, 0.0))
    if (hits.count > 0) {
      var sorted = Sort.sort(hits, Sorter.new(transform.worldPosition))
      if (sorted[0].gameObject.name == "playerBody") sorted.removeAt(0)

      if (sorted.count > 0) {
        _onTheGround = true
      }
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
      _onWall = false
      _velocity = Vec3.new(-_onWallDir.x, 1.0, -_onWallDir.z) * _speedWall
      _playerRotation.y = _onWallRotation - 180
      wasOnWall = false
    }

    if (wasOnWall && !_onWall) {
      _velocity.y = Math.min(_velocity.y, _jumpHeight)
    }

    // Find out how badly we need to move.
    var sin = -Math.sin(Math.deg2Rad * _playerRotation.y)
    var cos =  Math.cos(Math.deg2Rad * _playerRotation.y)

    var movement = Vec2.new(sin, -cos).normalized * movementVertical //+ Vec2.new(-cos, -sin).normalized * -movementHorizontal

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

    if (!_onWall) {
      _avgVelocity = vecLerp(_avgVelocity, _velocity, 0.1)
      var val = _avgVelocity.length * (1.0 / (Time.fixedDeltaTime / Time.timeScale))
      val = (val - (_speedWalk * 10)) / ((_speedSprint * 10) - (_speedWalk * 10))
      val = Math.clamp(val)
      _fovGoto = Math.lerp(_fovWalk, _fovRun, val)
    }
  }
}
