import "Core" for Vec2, Vec3, Vec4, Quat
import "Core" for Texture, Shader, Mesh
import "Core" for GameObject, Transform, Camera, Lod, RigidBody, WaveSource, Collider, MonoBehaviour, Light, MeshRender
import "Core" for PhysicsConstraints, Physics, Manifold
import "Core" for Input, Keys, Buttons, Axes
import "Core" for Math, Time, Debug, Sort, Console, World
import "Core" for PostProcess, LightTypes, ShadowTypes

import "resources/scripts/wren/input_controller" for InputController
import "resources/scripts/wren/physics_layers"   for PhysicsLayers
import "resources/scripts/wren/camera"           for Sorter
import "resources/scripts/wren/post_processor"   for PostProcessor
import "resources/scripts/wren/meshes"           for Meshes

class Serializer {
  construct new() {
    _values = {}
  }

  [name]=(value) { 
    if (value.type == Vec2) return _values[name] = Vec2.new(value.x, value.y)
    if (value.type == Vec3) return _values[name] = Vec3.new(value.x, value.y, value.z)
    if (value.type == Vec4) return _values[name] = Vec4.new(value.x, value.y, value.z, value.w)
    if (value.type == Quat) return _values[name] = Quat.new(value.x, value.y, value.z, value.w)
    _values[name] = value 
  }
  [name] { 
    var value = _values[name]
    if (value.type == Vec2) return Vec2.new(value.x, value.y)
    if (value.type == Vec3) return Vec3.new(value.x, value.y, value.z)
    if (value.type == Vec4) return Vec4.new(value.x, value.y, value.z, value.w)
    if (value.type == Quat) return Quat.new(value.x, value.y, value.z, value.w)

    return value
  }
}

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

  serialize(serializer) {
    serializer["arm1_shoulder_local_position"] = _arm1.shoulder.transform.localPosition
    serializer["arm1_shoulder_local_rotation"] = _arm1.shoulder.transform.localRotation
    serializer["arm2_shoulder_local_position"] = _arm2.shoulder.transform.localPosition
    serializer["arm2_shoulder_local_rotation"] = _arm2.shoulder.transform.localRotation
    serializer["leg1_shoulder_local_position"] = _leg1.shoulder.transform.localPosition
    serializer["leg1_shoulder_local_rotation"] = _leg1.shoulder.transform.localRotation
    serializer["leg2_shoulder_local_position"] = _leg2.shoulder.transform.localPosition
    serializer["leg2_shoulder_local_rotation"] = _leg2.shoulder.transform.localRotation
    serializer["arm1_elbow_local_position"]    = _arm1.elbow.transform.localPosition
    serializer["arm1_elbow_local_rotation"]    = _arm1.elbow.transform.localRotation
    serializer["arm2_elbow_local_position"]    = _arm2.elbow.transform.localPosition
    serializer["arm2_elbow_local_rotation"]    = _arm2.elbow.transform.localRotation
    serializer["leg1_elbow_local_position"]    = _leg1.elbow.transform.localPosition
    serializer["leg1_elbow_local_rotation"]    = _leg1.elbow.transform.localRotation
    serializer["leg2_elbow_local_position"]    = _leg2.elbow.transform.localPosition
    serializer["leg2_elbow_local_rotation"]    = _leg2.elbow.transform.localRotation
  }
  
  deserialize(serializer) {
    _arm1.shoulder.transform.localPosition = serializer["arm1_shoulder_local_position"]
    _arm1.shoulder.transform.localRotation = serializer["arm1_shoulder_local_rotation"]
    _arm2.shoulder.transform.localPosition = serializer["arm2_shoulder_local_position"]
    _arm2.shoulder.transform.localRotation = serializer["arm2_shoulder_local_rotation"]
    _leg1.shoulder.transform.localPosition = serializer["leg1_shoulder_local_position"]
    _leg1.shoulder.transform.localRotation = serializer["leg1_shoulder_local_rotation"]
    _leg2.shoulder.transform.localPosition = serializer["leg2_shoulder_local_position"]
    _leg2.shoulder.transform.localRotation = serializer["leg2_shoulder_local_rotation"]
    _arm1.elbow.transform.localPosition    = serializer["arm1_elbow_local_position"]
    _arm1.elbow.transform.localRotation    = serializer["arm1_elbow_local_rotation"]
    _arm2.elbow.transform.localPosition    = serializer["arm2_elbow_local_position"]
    _arm2.elbow.transform.localRotation    = serializer["arm2_elbow_local_rotation"]
    _leg1.elbow.transform.localPosition    = serializer["leg1_elbow_local_position"]
    _leg1.elbow.transform.localRotation    = serializer["leg1_elbow_local_rotation"]
    _leg2.elbow.transform.localPosition    = serializer["leg2_elbow_local_position"]
    _leg2.elbow.transform.localRotation    = serializer["leg2_elbow_local_rotation"]
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
  
  serialize(serializer) {
    serializer["velocity"]   = _velocity
    serializer["on_wall"]    = _onWall
    serializer["in_air"]     = _inAir
    serializer["arm_length"] = _armLength
    serializer["total_time"] = _totalTime

    var superSerializer = Serializer.new()
    super.serialize(superSerializer)
    serializer["super"] = superSerializer
  }
  
  deserialize(serializer) {
    _velocity  = serializer["velocity"]
    _onWall    = serializer["on_wall"]
    _inAir     = serializer["in_air"]
    _armLength = serializer["arm_length"]
    _totalTime = serializer["total_time"]

    super.deserialize(serializer["super"])
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

  serialize(serializer) {
    serializer["has_input"]       = _hasInput
    serializer["sensitivity"]     = _sensitivity
    serializer["jump_height"]     = _jumpHeight
    serializer["player_rotation"] = _playerRotation
    serializer["velocity"]        = _velocity
    serializer["avg_velocity"]    = _avgVelocity
    serializer["on_the_ground"]   = _onTheGround
    serializer["cam_point"]       = _camPoint
    serializer["follow_distance"] = _followDistance
    serializer["mod"]             = _mod
    serializer["held_jump"]       = _heldJump
    
    serializer["rigid_body_velocity"]            = _rigidBody.velocity
    serializer["rigid_body_angular_constraints"] = _rigidBody.angularConstraints

    serializer["speed_walk"]       = _speedWalk
    serializer["speed_sprint"]     = _speedSprint
    serializer["speed_wall"]       = _speedWall
    serializer["max_speed_walk"]   = _maxSpeedWalk
    serializer["max_speed_sprint"] = _maxSpeedSprint

    serializer["fov_walk"] = _fovWalk
    serializer["fov_run"]  = _fovRun
    serializer["fov_wall"] = _fovWalk
    serializer["fov"]      = _fov
    serializer["fov_goto"] = _fovGoto

    serializer["on_wall"]              = _onWall
    serializer["on_wall_rotation"]     = _onWallRotation
    serializer["on_wall_dir"]          = _onWallDir
    serializer["on_wall_last_pressed"] = _onWallLastPressed

    serializer["local_position"]            = transform.localPosition
    serializer["local_rotation"]            = transform.localRotation
    serializer["camera_local_position"]     = _transformCamera.localPosition
    serializer["camera_local_rotation"]     = _transformCamera.localRotation
    serializer["in_between_local_position"] = _transformInBetween.localPosition
    serializer["in_between_local_rotation"] = _transformInBetween.localRotation
    serializer["rigid_body_local_position"] = _rigidBody.gameObject.transform.localPosition
    serializer["rigid_body_local_rotation"] = _rigidBody.gameObject.transform.localRotation
    
    var hoomanSerializer = Serializer.new()
    _hooman.serialize(hoomanSerializer)
    serializer["hooman"] = hoomanSerializer
  }
  
  deserialize(serializer) {
    _hasInput       = serializer["has_input"]      
    _sensitivity    = serializer["sensitivity"]    
    _jumpHeight     = serializer["jump_height"]    
    _playerRotation = serializer["player_rotation"]
    _velocity       = serializer["velocity"]       
    _avgVelocity    = serializer["avg_velocity"]   
    _onTheGround    = serializer["on_the_ground"]  
    _camPoint       = serializer["cam_point"]      
    _followDistance = serializer["follow_distance"]
    _mod            = serializer["mod"]            
    _heldJump       = serializer["held_jump"]      
    
    _rigidBody.velocity           = serializer["rigid_body_velocity"]           
    _rigidBody.angularConstraints = serializer["rigid_body_angular_constraints"]

    _speedWalk      = serializer["speed_walk"]      
    _speedSprint    = serializer["speed_sprint"]    
    _speedWall      = serializer["speed_wall"]      
    _maxSpeedWalk   = serializer["max_speed_walk"]  
    _maxSpeedSprint = serializer["max_speed_sprint"]

    _fovWalk = serializer["fov_walk"]
    _fovRun  = serializer["fov_run"] 
    _fovWalk = serializer["fov_wall"]
    _fov     = serializer["fov"]     
    _fovGoto = serializer["fov_goto"]

    _onWall            = serializer["on_wall"]             
    _onWallRotation    = serializer["on_wall_rotation"]    
    _onWallDir         = serializer["on_wall_dir"]         
    _onWallLastPressed = serializer["on_wall_last_pressed"]

    transform.localPosition                       = serializer["local_position"]           
    transform.localRotation                       = serializer["local_rotation"]           
    _transformCamera.localPosition                = serializer["camera_local_position"]    
    _transformCamera.localRotation                = serializer["camera_local_rotation"]    
    _transformInBetween.localPosition             = serializer["in_between_local_position"]
    _transformInBetween.localRotation             = serializer["in_between_local_rotation"]
    _rigidBody.gameObject.transform.localPosition = serializer["rigid_body_local_position"]
    _rigidBody.gameObject.transform.localRotation = serializer["rigid_body_local_rotation"]
    
    _hooman.deserialize(serializer["hooman"])
  }

  updateMesh() {
    _hooman.inAir    = !_onTheGround
    _hooman.onWall   = _onWall
    _hooman.velocity = _velocity

    _hooman.fixedUpdate()
    
    var lastDoSerialize = _doSerialize
    _doSerialize = (InputController.Serialize > 0.0 ? true : false)
    if (_doSerialize && !lastDoSerialize) {
      World.serialize("serialized")
      //serialize(_serializer)
    }

    var lastDoDeserialize = _doDeserialize
    _doDeserialize = (InputController.Deserialize > 0.0 ? true : false)
    if (_doDeserialize && !lastDoDeserialize) {
      World.deserialize("serialized")
      //deserialize(_serializer)
    }
  }

  initialize() {
    gameObject.name         = "player"
    transform.localPosition = Vec3.new(0.0, 2.0, 0.0)

    _hooman = Hooman.new(gameObject)
    _serializer = Serializer.new()

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
    transform.worldPosition = _rigidBody.gameObject.transform.worldPosition - Vec3.new(0.0, -1.0, 0.0)
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
