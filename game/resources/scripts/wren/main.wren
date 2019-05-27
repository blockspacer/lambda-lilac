import "Core" for Vec2, Vec3, Vec4
import "Core" for Texture, Shader, Mesh
import "Core" for GameObject, Transform, Camera, MeshRender, Lod, RigidBody, WaveSource, Collider, MonoBehaviour
import "Core" for Input, Keys, Buttons, Axes
import "Core" for Math, Graphics, GUI, Time, File, Assert, PostProcess, Console, Physics, Debug, Sort, PhysicsConstraints
import "Core" for NavMesh

import "resources/scripts/wren/post_processor" for PostProcessor
import "resources/scripts/wren/input_controller" for InputController
import "resources/scripts/wren/camera" for FreeLookCamera, Sorter
import "resources/scripts/wren/player" for ThirdPersonCamera, Hooman, HoomanBase
import "resources/scripts/wren/lighting" for Lighting
import "resources/scripts/wren/trees" for Trees
import "resources/scripts/wren/item_manager" for ItemManager
import "resources/scripts/wren/door" for Door
import "resources/scripts/wren/physics_layers" for PhysicsLayers
//import "resources/scripts/wren/node_map" for Node, NodeMap, NodeEditor
import "resources/scripts/wren/city" for City
import "resources/scripts/wren/meshes" for Meshes

class Rando is MonoBehaviour {
  construct new()      { super()                     }
  static goGet(val)    { MonoBehaviour.goGet(val)    }
  static goRemove(val) { MonoBehaviour.goRemove(val) }

  static updateHooman() {
    if (__hooman) {
      __hooman.velocity = Vec3.new(__speed, 0.0, 0.0)
      __hooman.fixedUpdate()
    }
  }

  realInitialize(city) {
    if (__hooman == null) __hooman = Hooman.newNoMesh()

    _city = city
    _user = Vec3.new(0.0)
    _notMoved = 0.0
    _prevPos = Vec3.new(0.0)
    __maxRange = 175
    __minRange = 150

    gameObject.name = "rando"
    transform.worldScale = Vec3.new(1.0)

    var x = Math.random(-_city.numBlocks.x / 2, _city.numBlocks.x / 2 - 1).round * _city.blockSize.x
    var z = Math.random(-_city.numBlocks.y / 2, _city.numBlocks.y / 2 - 1).round * _city.blockSize.y
    transform.worldPosition = Vec3.new(x, 2, z)

    _hooman = HoomanBase.new(gameObject)

    var entityCollider = GameObject.new()
    entityCollider.transform.localScale = Vec3.new(1.0, 3.0, 1.0)
    entityCollider.transform.parent = gameObject
    var collider = entityCollider.addComponent(Collider)
    _rigidBody = entityCollider.addComponent(RigidBody)
    collider.makeCapsuleCollider()
    collider.layers    = PhysicsLayers.General | PhysicsLayers.MovingObjects
    _rigidBody.angularConstraints = PhysicsConstraints.X | PhysicsConstraints.Y | PhysicsConstraints.Z
    _rigidBody.mass     = 1.0
    _rigidBody.friction = 2.0
    __speed = 7.5
  }

  user=(v) { _user = v }

  getNextNode() {
    if (_positionList == null || _positionList.isEmpty) {
      _positionList = null
      _currPosition = null
      return
    }

    _currPosition = _positionList[0]
    _positionList.removeAt(0)
  }

  getPath() {
    var x = (Math.random(_user.x - __maxRange, _user.x + __maxRange) / _city.blockSize.x).round * _city.blockSize.x
    var z = (Math.random(_user.z - __maxRange, _user.z + __maxRange) / _city.blockSize.y).round * _city.blockSize.y
    getPath(Vec3.new(x, 0.0, z))
  }
  
  getPath(to) {
    _promise = _city.navMesh.findPathPromise(transform.worldPosition, to)
    //_positionList = _city.navMesh.findPath(transform.worldPosition, to)
  }

  fixedUpdate() {
    /*if (_positionList == null || _currPosition == null)*/ {
      // Keep the rando near the player.
      var diff = Vec2.new(transform.worldPosition.x - _user.x, transform.worldPosition.z - _user.z)
      if (diff.lengthSqr > __maxRange * __maxRange) {
        var angle = Math.random(0.0, Math.tau)
        var dir = Vec2.new(Math.sin(angle), Math.cos(angle)) * __minRange
        var pos = Vec2.new(_user.x, _user.z) + dir
        var x = (pos.x / _city.blockSize.x).round * _city.blockSize.x
        var z = (pos.y / _city.blockSize.y).round * _city.blockSize.y
        transform.worldPosition = Vec3.new(x, 2, z)
        _positionList = null
        _currPosition = null
        return
      }

      if (transform.worldPosition.y < -20) {
        transform.worldPosition = Vec3.new(0.0, 2.0, 0.0)
        _positionList = null
        _currPosition = null
        return
      }

      if (_promise != null) {
        if (_promise.finished) {
          _positionList = _promise.path
          _promise = null
        } else {
          return
        }
      } else {
        if (_currPosition == null && _positionList == null) {
          getPath()
        }
      }

      if (_currPosition == null) {
        getNextNode()
        if (_currPosition == null) {
          //_rigidBody.velocity = Vec3.new(0.0, _rigidBody.velocity.y, 0.0)
          return
        }
      }
    }

    simpleMove()
  }

  simpleMove() {
    _velocity = _rigidBody.velocity
    
    var offset = _currPosition - transform.worldPosition
    offset.y = 0.0
    var length = offset.length
    var movement = (length < __speed) ? length : __speed

    if (length < 1.0) {
      getNextNode()
      return
    }

    _velocity = _velocity + offset.normalized * movement
    gameObject.transform.worldPosition = _rigidBody.gameObject.transform.worldPosition - Vec3.new(0.0, -1.0, 0.0)
    _rigidBody.gameObject.transform.localPosition = Vec3.new(0.0, -1.0, 0.0)
    gameObject.transform.worldEuler = Vec3.new(0.0, Math.atan2(-_velocity.z, _velocity.x) - Math.pi / 2, 0.0)

    __hooman.clone(_hooman)
    
    var delta = transform.worldPosition - _prevPos
    if (delta.length < 0.05) {
      _notMoved = _notMoved + Time.fixedDeltaTime
    }
    _prevPos = transform.worldPosition

    if (_notMoved > 1.0) {
      _notMoved = 0.0
      getPath()
    }
    
    // If we're on the ground then we should slow down immensly quickly.
    //_velocity.x = _velocity.x * (1.0 - Time.fixedDeltaTime * 5.0)
    //_velocity.z = _velocity.z * (1.0 - Time.fixedDeltaTime * 5.0)
    
    var len = Vec2.new(_velocity.x, _velocity.z).length
    if (len > __speed) {
      _velocity.x = (_velocity.x / len) * __speed
      _velocity.z = (_velocity.z / len) * __speed
    }

    _rigidBody.velocity = _velocity
  }
  
  draw() {
    if (_currPosition) {
      Debug.drawLine(_currPosition - Vec3.new(1.0, 0.0, 0.0), _currPosition + Vec3.new(1.0, 0.0, 0.0), Vec4.new(1.0, 0.0, 0.0, 1.0))
      Debug.drawLine(_currPosition - Vec3.new(0.0, 1.0, 0.0), _currPosition + Vec3.new(0.0, 1.0, 0.0), Vec4.new(1.0, 0.0, 0.0, 1.0))
      Debug.drawLine(_currPosition - Vec3.new(0.0, 0.0, 1.0), _currPosition + Vec3.new(0.0, 0.0, 1.0), Vec4.new(1.0, 0.0, 0.0, 1.0))
      Debug.drawLine(transform.worldPosition, _currPosition, Vec4.new(0.0, 1.0, 0.0, 1.0))
    }

    if (_positionList != null && _positionList.count >= 1) {
      var prev = _positionList[0]
      
      if (_currPosition) {
        prev = _currPosition
      }

      for (i in 0..._positionList.count) {
        Debug.drawLine(prev, _positionList[i], Vec4.new(0.0, 0.0, 1.0, 1.0))
        Debug.drawLine(_positionList[i] - Vec3.new(1.0, 0.0, 0.0), _positionList[i] + Vec3.new(1.0, 0.0, 0.0), Vec4.new(1.0, 0.0, 0.0, 1.0))
        Debug.drawLine(_positionList[i] - Vec3.new(0.0, 1.0, 0.0), _positionList[i] + Vec3.new(0.0, 1.0, 0.0), Vec4.new(1.0, 0.0, 0.0, 1.0))
        Debug.drawLine(_positionList[i] - Vec3.new(0.0, 0.0, 1.0), _positionList[i] + Vec3.new(0.0, 0.0, 1.0), Vec4.new(1.0, 0.0, 0.0, 1.0))
        prev = _positionList[i]
      }
    }
  }
}

class World {
  construct new() {
  }

  constructNodes() {
    _nodeMap = NavMesh.new()
    _toggleNodeEditor = false
    //var nodeEditor = NodeEditor.new(_nodeMap)
    //nodeEditor.deserialize()
  }

  getNextNode() {
    if (_positionList == null || _positionList.isEmpty) {
      return
    }

    _currPosition = _positionList[0]
    _positionList.removeAt(0)
  }

  moveMoveMe() {
    if (!_moveMe) {
      return
    }

    if (_currPosition == null && (_positionList == null || _positionList.count <= 0)) {
      _positionList = _nodeMap.findPath(_moveMe.transform.worldPosition, _camera.transform.worldPosition)
      return
    }

    if (_currPosition == null) {
      getNextNode()
    }

    var offset = _currPosition - _moveMe.transform.worldPosition
    offset.y = 0.0
    var length = offset.length
    var movement = 2.5 * Time.fixedDeltaTime
    if (length < movement) {
      movement = length
    }

    if (length < 0.1) {
      var currPosition = _currPosition
      _positionList = _nodeMap.findPath(_moveMe.transform.worldPosition, _camera.transform.worldPosition)
      getNextNode()
      if (_currPosition == currPosition) {
        getNextNode()
      }
      //_currNode = _currNode.connections[Math.random(_currNode.connections.count - 1).round]
    }

    offset = offset.normalized * movement
    _moveMe.transform.worldPosition = _moveMe.transform.worldPosition + offset
  }

  initializeCity() {
    _city = City.new(Vec2.new(10), Vec2.new(100), Vec2.new(25), Vec2.new(10), 50, 100)
    _randos = []

  for (i in 0...100) {
      var rando = GameObject.new()
      rando.addComponent(Rando).realInitialize(_city)
      _randos.add(rando)
    }
  }

  initializeRoom() {
    constructNodes()

    _item_manager = GameObject.new().addComponent(ItemManager)

    _door = GameObject.new()
    _door.addComponent(Door)

    // Move the camera so you do not spawn under the ground.
    var new_position = _camera.transform.worldPosition
    new_position.y   = 2.0
    _camera.transform.worldPosition = new_position

    _moveMe = _item_manager.cube(Vec3.new(0.0), Vec3.new(1.0))

    _item_manager.cube(Vec3.new(-11.0, 9.0, 10.0), Vec3.new(2.0))
    _item_manager.cube(Vec3.new(-10.0, 7.0, 10.0), Vec3.new(2.5))
    _item_manager.cube(Vec3.new(-10.0, 9.0,  9.0), Vec3.new(4.0, 0.5, 2.00))
    //_item_manager.spear(Vec3.new(2.0, 9.0, 0.0))

    // Top floor torches.
    _item_manager.torch(Vec3.new( 10.0, 6.0,  12.25), Vec3.new(0.5), Vec3.new(0.0, 0.0 * Math.deg2Rad, 0.0))
    _item_manager.torch(Vec3.new(  0.0, 6.0,  12.25), Vec3.new(0.5), Vec3.new(0.0, 0.0 * Math.deg2Rad, 0.0))
    _item_manager.torch(Vec3.new(-10.0, 6.0,  12.25), Vec3.new(0.5), Vec3.new(0.0, 0.0 * Math.deg2Rad, 0.0))
    _item_manager.torch(Vec3.new( 10.0, 6.0, -12.25), Vec3.new(0.5), Vec3.new(0.0, 180.0 * Math.deg2Rad, 0.0))
    _item_manager.torch(Vec3.new(  0.0, 6.0, -12.25), Vec3.new(0.5), Vec3.new(0.0, 180.0 * Math.deg2Rad, 0.0))
    _item_manager.torch(Vec3.new(-10.0, 6.0, -12.25), Vec3.new(0.5), Vec3.new(0.0, 180.0 * Math.deg2Rad, 0.0))

    // Bottom floor torches.
    _item_manager.torch(Vec3.new( 10.0, 1.0,  12.25), Vec3.new(0.5), Vec3.new(0.0, 0.0 * Math.deg2Rad, 0.0))
    _item_manager.torch(Vec3.new(  0.0, 1.0,  12.25), Vec3.new(0.5), Vec3.new(0.0, 0.0 * Math.deg2Rad, 0.0))
    _item_manager.torch(Vec3.new(-10.0, 1.0,  12.25), Vec3.new(0.5), Vec3.new(0.0, 0.0 * Math.deg2Rad, 0.0))
    _item_manager.torch(Vec3.new( 10.0, 1.0, -12.25), Vec3.new(0.5), Vec3.new(0.0, 180.0 * Math.deg2Rad, 0.0))
    _item_manager.torch(Vec3.new(  0.0, 1.0, -12.25), Vec3.new(0.5), Vec3.new(0.0, 180.0 * Math.deg2Rad, 0.0))
    _item_manager.torch(Vec3.new(-10.0, 1.0, -12.25), Vec3.new(0.5), Vec3.new(0.0, 180.0 * Math.deg2Rad, 0.0))

    // Misc torches.
    _item_manager.torch(Vec3.new(12.25, 1.5, 0.0), Vec3.new(0.5), Vec3.new(0.0, 90.0 * Math.deg2Rad, 0.0))

    {
      var model = GameObject.new()
      model.transform.worldScale = Vec3.new(2.5)
      //model.transform.worldPosition = Vec3.new(0.0, 0.0, -1.0)
      var mesh = Mesh.load("resources/gltf/lvl/lvl.gltf")
      //var mesh = Mesh.load("resources/gltf/san_miguel_low_poly.glb")
      model.addComponent(MeshRender).attach(mesh)
      var collider = model.addComponent(Collider)
      model.getComponent(MeshRender).makeStaticRecursive()
      collider.makeMeshColliderRecursive(mesh)
      collider.layersRecursive = PhysicsLayers.General |/*|*/ PhysicsLayers.MovingObjects

      new_position.y = 2.0
      _camera.transform.worldPosition = new_position
    }
  }

  delayedInitialize() {
    initializeCity()
    //initializeRoom()

    GUI.executeJavaScript("LoadWebPage('file:///resources/web-pages/menu.html')")
    GUI.bindCallback("changedSetting(_,_)", this)
    GUI.bindCallback("movedToMenu(_)", this)
  }

  initialize() {
    _post_processer = PostProcessor.new()
    _lighting = Lighting.new()

    _camera = GameObject.new()
    _camera.addComponent(ThirdPersonCamera)

    GUI.loadURL("file:///resources/web-pages/ui.html")
    GUI.executeJavaScript("LoadWebPage('file:///resources/web-pages/loading_screen.html')")

    //delayedInitialize()
    _frameIndex = -1
  }
  deinitialize() {

  }
  update() {
    _frameIndex = _frameIndex + 1
    if (_frameIndex == 0) return
    if (_frameIndex == 1) return delayedInitialize()


    GUI.executeJavaScript("if (gameState == Game) { updateDebug(%(Time.deltaTime)) }")
    
    // Update the RSM light.
    var sin_y_rot = -Math.sin(Math.deg2Rad * _camera.getComponent(FreeLookCamera).rotation.y)
    var cos_y_rot = -Math.cos(Math.deg2Rad * _camera.getComponent(FreeLookCamera).rotation.y)
    var offset = Vec3.new(sin_y_rot, 0.0, cos_y_rot).normalized * 12.5
    var transform = _lighting.rsm.getComponent(Transform)
    transform.worldPosition = _camera.transform.worldPosition + offset

    if (_city) {
      //_city.drawTris()
      for (rando in _randos) {
        rando.getComponent(Rando).user = _camera.transform.worldPosition
        //rando.getComponent(Rando).draw()
      }

      return
    }

    var toggleNodeEditor = InputController.ToggleNodeEditor != 0
    var toggleEditor = toggleNodeEditor && !_toggleNodeEditor
    _toggleNodeEditor = toggleNodeEditor		

    if (toggleEditor) {
      if (_nodeEditor == null) {
        _nodeEditor = null// NodeEditor.new(_nodeMap)
        Time.timeScale = 0.1
      } else {
        _nodeEditor = null
        Time.timeScale = 1.0
      }
    }

    if (_nodeEditor) {
      var camera = _camera.getComponent(FreeLookCamera)
      var from = camera.cameraTransform.worldPosition
      var selected = from - Vec3.new(0.0, 1.0, 0.0)
      var to = from + camera.camera.ndcToWorld(InputController.MousePosition) * 25
      var sorted = Sort.sort(Physics.castRay(from, to), Sorter.new(from))
      if (sorted.count > 0) {
        selected = sorted[0].point
      }

      _nodeEditor.update(selected)
      _nodeMap.draw()
    }
  }

  fixedUpdate() {
    moveMoveMe()
    Rando.updateHooman()
  }

  changedSetting(id, val) {
    if (id == "vsync") {
      Graphics.vsync = val > 0.0 ? true : false
    }
    if (id == "render_scale") {
      Graphics.renderScale = val
    }
    if (id == "physics_debug_draw") {
      Physics.debugDrawEnabled = val > 0.0 ? true : false
    }
  }

  movedToMenu(menu) {
    _camera.getComponent(ThirdPersonCamera).hasInput = menu == "" ? true : false
  }
}
