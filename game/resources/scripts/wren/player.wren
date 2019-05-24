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
  construct newNoMesh(parent) {
    _shoulder = GameObject.new()
    _shoulder.transform.parent        = parent
    _shoulder.transform.localPosition = Vec3.new(0.0, 0.0, -0.5)
    _elbow = GameObject.new()
    _elbow.transform.parent        = _shoulder
    _elbow.transform.localPosition = Vec3.new(0.0, 0.0, -1.0)
  }

  construct new(parent) {
    _shoulder = GameObject.new()
    _shoulder.transform.parent        = parent
    _shoulder.transform.localPosition = Vec3.new(0.0, 0.0, -0.5)
    
    _elbow = GameObject.new()
    _elbow.transform.parent        = _shoulder
    _elbow.transform.localPosition = Vec3.new(0.0, 0.0, -1.0)
    
    var upperArm = GameObject.new()
    upperArm.transform.parent = _shoulder
    var meshRender     = upperArm.addComponent(MeshRender)
    meshRender.mesh    = Meshes.cube
    meshRender.subMesh = 0
    upperArm.transform.localScale    = Vec3.new(0.25, 0.25, 1.0)
    upperArm.transform.localPosition = Vec3.new(0.0, 0.0, -0.5)
    
    var lowerArm = GameObject.new()
    lowerArm.transform.parent = _elbow
    meshRender         = lowerArm.addComponent(MeshRender)
    meshRender.mesh    = Meshes.cube
    meshRender.subMesh = 0
    lowerArm.transform.localScale    = Vec3.new(0.25, 0.25, 1.0)
    lowerArm.transform.localPosition = Vec3.new(0.0, 0.0, -0.5)
  }

  shoulder { _shoulder }
  elbow    { _elbow }
}

class HoomanBase {
  construct newNoMesh() {
    _core = GameObject.new()
    _arm1 = Arm.newNoMesh(_core)
    _arm2 = Arm.newNoMesh(_core)
    _leg1 = Arm.newNoMesh(_core)
    _leg2 = Arm.newNoMesh(_core)

    _arm1.shoulder.transform.localPosition = Vec3.new( -0.5,  1.5, 0.0)
    _arm2.shoulder.transform.localPosition = Vec3.new(  0.5,  1.5, 0.0)
    _leg1.shoulder.transform.localPosition = Vec3.new(-0.25, -0.5, 0.0)
    _leg2.shoulder.transform.localPosition = Vec3.new( 0.25, -0.5, 0.0)
  }

  construct new(parent) {
    var mesh = Meshes.cube
    _core = GameObject.new()
    if (parent) _core.transform.parent = parent
    
    var body = GameObject.new()
    body.transform.parent = _core
    var meshRender     = body.addComponent(MeshRender)
    meshRender.mesh    = mesh
    meshRender.subMesh = 0
    body.transform.localScale = Vec3.new(1.0, 2.0, 0.5)
    body.transform.localPosition = Vec3.new(0.0, 0.5, 0.0)
    
    _neck = GameObject.new()
    _neck.transform.parent = _core
    _neck.transform.localPosition = Vec3.new(0.0, 1.5, 0.0)
    
    var head = GameObject.new()
    head.transform.parent = _neck
    meshRender         = head.addComponent(MeshRender)
    meshRender.mesh    = mesh
    meshRender.subMesh = 0
    head.transform.localScale = Vec3.new(0.75)
    head.transform.localPosition = Vec3.new(0.0, 0.5, 0.0)

    _arm1 = Arm.new(_core)
    _arm2 = Arm.new(_core)
    _leg1 = Arm.new(_core)
    _leg2 = Arm.new(_core)

    _arm1.shoulder.transform.localPosition = Vec3.new( -0.5,  1.5, 0.0)
    _arm2.shoulder.transform.localPosition = Vec3.new(  0.5,  1.5, 0.0)
    _leg1.shoulder.transform.localPosition = Vec3.new(-0.25, -0.5, 0.0)
    _leg2.shoulder.transform.localPosition = Vec3.new( 0.25, -0.5, 0.0)
  }
  
  arm1 { _arm1 }
  arm2 { _arm2 }
  leg1 { _leg1 }
  leg2 { _leg2 }
  core { _core }
  
  clone(other) {
    other.arm1.shoulder.transform.localRotation = _arm1.shoulder.transform.localRotation
    other.arm1.elbow.transform.localRotation    = _arm1.elbow.transform.localRotation
    other.arm2.shoulder.transform.localRotation = _arm2.shoulder.transform.localRotation
    other.arm2.elbow.transform.localRotation    = _arm2.elbow.transform.localRotation
    other.leg1.shoulder.transform.localRotation = _leg1.shoulder.transform.localRotation
    other.leg1.elbow.transform.localRotation    = _leg1.elbow.transform.localRotation
    other.leg2.shoulder.transform.localRotation = _leg2.shoulder.transform.localRotation
    other.leg2.elbow.transform.localRotation    = _leg2.elbow.transform.localRotation
    
    other.arm1.shoulder.transform.localPosition = _arm1.shoulder.transform.localPosition
    other.arm2.shoulder.transform.localPosition = _arm2.shoulder.transform.localPosition
    other.leg1.shoulder.transform.localPosition = _leg1.shoulder.transform.localPosition
    other.leg2.shoulder.transform.localPosition = _leg2.shoulder.transform.localPosition
  }
}

class Hooman is HoomanBase {
  construct newNoMesh() {
    super()

    _originArmLeft  = Vec3.new( -0.5,   1.5, 0.0)
    _originArmRight = Vec3.new(  0.5,   1.5, 0.0)
    _originLegLeft  = Vec3.new(-0.25,  -0.5, 0.0)
    _originLegRight = Vec3.new( 0.25,  -0.5, 0.0)
    _pointArmLeft   = Vec3.new(-0.75, -0.25, 0.0)
    _pointArmRight  = Vec3.new( 0.75, -0.25, 0.0)
    _pointLegLeft   = Vec3.new(-0.25,  -3.5, 0.0)
    _pointLegRight  = Vec3.new( 0.25,  -3.5, 0.0)

    _armLength = 2.0
    _totalTime = 0.0
  }
  construct new(parent) {
    super(parent)

    _originArmLeft  = Vec3.new( -0.5,   1.5, 0.0)
    _originArmRight = Vec3.new(  0.5,   1.5, 0.0)
    _originLegLeft  = Vec3.new(-0.25,  -0.5, 0.0)
    _originLegRight = Vec3.new( 0.25,  -0.5, 0.0)
    _pointArmLeft   = Vec3.new(-0.75, -0.25, 0.0)
    _pointArmRight  = Vec3.new( 0.75, -0.25, 0.0)
    _pointLegLeft   = Vec3.new(-0.25,  -3.5, 0.0)
    _pointLegRight  = Vec3.new( 0.25,  -3.5, 0.0)

    _armLength = 2.0
    _totalTime = 0.0
  }

  lookRotation(dir) {
    if (dir.normalized == Vec3.new(0.0, 1.0, 0.0)) {
      return Math.lookRotation(dir, Vec3.new(0.0, 0.0, 1.0))
    } else {
      return Math.lookRotation(dir, Vec3.new(0.0, 1.0, 0.0))
    }
  }

  fixArm(from, to, maxLength, crossDir, arm) {
    var oldTo = to
    var delta = to - from
    var length = delta.length
    if (length > maxLength) {
      to = from + (to - from).normalized * maxLength
      length = maxLength
    }
    var perpendicular = length < maxLength ? delta.normalized.cross(crossDir).normalized * Math.sqrt((maxLength * 0.5) * (maxLength * 0.5) - (length * 0.5) * (length * 0.5)) : Vec3.new(0.0)

    var center = (from + to) * 0.5 + perpendicular
    arm.shoulder.transform.worldPosition = from
    arm.shoulder.transform.worldRotation = lookRotation(from - center)
    arm.elbow.transform.worldRotation    = lookRotation(center - to)
  }
  
  rotateVector(a, b) {
    var r = Math.atan2(a.y, a.x) + Math.atan2(b.y, b.x)
    return Vec2.new(Math.cos(r), Math.sin(r)) * a.length
  }
  
  rotateVector3D(a, b) {
    var t = rotateVector(Vec2.new(a.x, a.z), b)
    return Vec3.new(t.x, a.y, t.y)
  }

  fixedUpdate() {
    if (_inAir) {
      if (_onWall) {
        animOnWall()
      } else {
        animInAir()
      }
    } else {
      animOnGround()
    }

    animate()
  }

  animIdle() {
    _pointArmLeft  = Vec3.new( -0.8, -0.25, 0.25)
    _pointArmRight = Vec3.new(  0.8, -0.25, 0.25)
    _pointLegLeft  = Vec3.new(-0.25,  -3.5,  0.0)
    _pointLegRight = Vec3.new( 0.25,  -3.5,  0.0)
  }

  animOnGround() {
    var velLength = Vec2.new(_velocity.x, _velocity.z).length
    if (velLength < 0.25) return animIdle()

    _totalTime = _totalTime + Time.fixedDeltaTime * velLength * (core.transform.worldForward.dot(_velocity) < 0.0 ? -1.0 : 1.0)

    _pointArmLeft.x  =  Math.abs(Math.sin(_totalTime)) * 0.15 - 0.8
    _pointArmLeft.y  = -0.25
    _pointArmLeft.z  = -Math.sin(_totalTime) * 0.5 + 0.25
    
    _pointArmRight.x = -Math.abs(Math.sin(_totalTime)) * 0.15 + 0.8
    _pointArmRight.y = -0.25
    _pointArmRight.z =  Math.sin(_totalTime) * 0.5 + 0.25
    
    _pointLegLeft.x  = -0.25
    _pointLegLeft.z  =  Math.sin(_totalTime)
    _pointLegLeft.y  =  Math.cos(_totalTime) * 0.5 - 2.0
    
    _pointLegRight.x =  0.25
    _pointLegRight.z = -Math.sin(_totalTime)
    _pointLegRight.y = -Math.cos(_totalTime) * 0.5 - 2.0
  }

  animOnWall() {
    _totalTime = _totalTime + Time.fixedDeltaTime * _velocity.y

    _pointArmLeft.x  = -1.25
    _pointArmLeft.y  =  Math.sin(_totalTime) + 1.0
    _pointArmLeft.z  =  0.5 - Math.cos(_totalTime) * 0.25
    
    _pointArmRight.x =  1.25
    _pointArmRight.y = -Math.sin(_totalTime) + 1.0
    _pointArmRight.z =  0.5 + Math.cos(_totalTime) * 0.25

    _pointLegLeft.x  = -0.5
    _pointLegLeft.y  =  Math.sin(_totalTime) - 1.5
    _pointLegLeft.z  =  0.5 - Math.cos(_totalTime) * 0.25
    
    _pointLegRight.x =  0.5
    _pointLegRight.y = -Math.sin(_totalTime) - 1.5
    _pointLegRight.z =  0.5 + Math.cos(_totalTime) * 0.25
  }
  
  animInAir() {
    _totalTime = _totalTime + Time.fixedDeltaTime * 10.0

    _pointArmLeft.x  = -1.5
    _pointArmLeft.y  =  (Math.sin(_totalTime) + Math.cos(_totalTime * 3.85) * 0.25) * 0.5 + 0.5
    _pointArmLeft.z  = -Math.cos(_totalTime) * 0.15 + 0.4
    
    _pointArmRight.x =  1.5
    _pointArmRight.y = -(Math.sin(_totalTime) + Math.cos(_totalTime * 3.85) * 0.25) * 0.5 + 0.5
    _pointArmRight.z =  Math.cos(_totalTime) * 0.15 + 0.4

    _pointLegLeft.x  = -0.5
    _pointLegLeft.y  =  (Math.sin(_totalTime) + Math.cos(_totalTime * 3.85) * 0.25) * 0.25 - 2.0
    _pointLegLeft.z  = -Math.cos(_totalTime) * 0.25
    
    _pointLegRight.x =  0.5
    _pointLegRight.y = -(Math.sin(_totalTime) + Math.cos(_totalTime * 3.85) * 0.25) * 0.25 - 2.0
    _pointLegRight.z =  Math.cos(_totalTime) * 0.25
  }

  animate() {
    var temp    = core.transform.worldRotation.mulVec3(Vec3.new(0.0, 0.0, 1.0))
    var forward = Vec2.new(temp.x, temp.z).normalized
    var right   = Vec2.new(-temp.z, temp.x).normalized

    var rpal = rotateVector3D(_pointArmLeft,   right)
    var rpar = rotateVector3D(_pointArmRight,  right)
    var rpll = rotateVector3D(_pointLegLeft,   right)
    var rplr = rotateVector3D(_pointLegRight,  right)
    var roal = rotateVector3D(_originArmLeft,  right)
    var roar = rotateVector3D(_originArmRight, right)
    var roll = rotateVector3D(_originLegLeft,  right)
    var rolr = rotateVector3D(_originLegRight, right)

    var armDirLeft  = Vec3.new(-right.x, 0.0, -right.y)
    var armDirRight = Vec3.new(-right.x, 0.0, -right.y)

    var center = core.transform.worldPosition
    fixArm(center + roal, center + rpal, _armLength, armDirLeft,  arm1)
    fixArm(center + roar, center + rpar, _armLength, armDirRight, arm2)
    fixArm(center + roll, center + rpll, _armLength, Vec3.new(right.x, 0.0, right.y), leg1)
    fixArm(center + rolr, center + rplr, _armLength, Vec3.new(right.x, 0.0, right.y), leg2)
  }

  velocity=(velocity) { _velocity = velocity }
  velocity { _velocity }
  inAir=(inAir) { _inAir = inAir }
  inAir { _inAir }
  onWall=(onWall) { _onWall = onWall }
  onWall { _onWall }
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
    _hooman.inAir = !_onTheGround
    _hooman.onWall = _onWall
    _hooman.velocity = _velocity

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
    _camPoint = Math.lerp3(_camPoint, _camPoint + delta * (length - _followDistance), 0.1)

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
      localPosition = Math.lerp3(_transformCamera.localPosition, Vec3.new(0.0, 1.0, 0.0), 0.01)
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
      _avgVelocity = Math.lerp3(_avgVelocity, _velocity, 0.1)
      var val = _avgVelocity.length * (1.0 / (Time.fixedDeltaTime / Time.timeScale))
      val = (val - (_speedWalk * 10)) / ((_speedSprint * 10) - (_speedWalk * 10))
      val = Math.clamp(val)
      _fovGoto = Math.lerp(_fovWalk, _fovRun, val)
    }
  }
}
