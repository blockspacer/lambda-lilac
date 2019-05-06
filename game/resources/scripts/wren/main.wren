import "Core/Vec2" for Vec2
import "Core/Vec3" for Vec3
import "Core/Vec4" for Vec4

import "Core/Texture" for Texture
import "Core/Shader" for Shader
import "Core/Mesh" for Mesh

import "Core/GameObject" for GameObject

import "Core/Transform" for Transform
import "Core/Camera" for Camera
import "Core/MeshRender" for MeshRender
import "Core/Lod" for Lod
import "Core/RigidBody" for RigidBody
import "Core/WaveSource" for WaveSource
import "Core/Collider" for Collider

import "Core/Input" for Input, Keys, Buttons, Axes
import "Core/Math" for Math

import "Core/Graphics" for Graphics
import "Core/GUI" for GUI
import "Core/Time" for Time
import "Core/File" for File
import "Core/Assert" for Assert

import "Core/MonoBehaviour" for MonoBehaviour

import "Core/PostProcess" for PostProcess
import "Core/Console" for Console
import "Core/Physics" for Physics
import "Core/Debug" for Debug
import "Core/Sort" for Sort

import "resources/scripts/wren/post_processor" for PostProcessor
import "resources/scripts/wren/input_controller" for InputController
import "resources/scripts/wren/camera" for FreeLookCamera, Sorter
import "resources/scripts/wren/lighting" for Lighting
import "resources/scripts/wren/trees" for Trees
import "resources/scripts/wren/item_manager" for ItemManager
import "resources/scripts/wren/door" for Door
import "resources/scripts/wren/physics_layers" for PhysicsLayers
import "resources/scripts/wren/node_map" for Node, NodeMap, NodeEditor

class CustomMesh {
  construct new () {
    _positions = []
    _normals   = []
    _tangents  = []
    _uvs       = []
    _indices   = []
    _baseIndex = 0
  }

  addTri(posA, posB, posC, uvA, uvB, uvC) {
   	var u = posB - posA
    var v = posC - posA
    var normal = Vec3.new(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x)

		var t1 = normal.cross(Vec3.new(1.0, 0.0, 0.0))
		var t2 = normal.cross(Vec3.new(0.0, 1.0, 0.0))
		var tangent = (t1.lengthSqr > t2.lengthSqr) ? t1 : t2

    _positions.add(posA)
    _positions.add(posB)
    _positions.add(posC)
    _normals.add(normal)
    _normals.add(normal)
    _normals.add(normal)
    _tangents.add(tangent)
    _tangents.add(tangent)
    _tangents.add(tangent)
    _uvs.add(uvA)
    _uvs.add(uvB)
    _uvs.add(uvC)
    _indices.add(_positions.count - 3)
    _indices.add(_positions.count - 2)
    _indices.add(_positions.count - 1)
    _baseIndex = _baseIndex + 3
  }

  addQuad(posA, posB, posC, posD, uvA, uvB, uvC, uvD) {
   	var u = posB - posA
    var v = posC - posA
    var normal = Vec3.new(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x)

		var t1 = normal.cross(Vec3.new(1.0, 0.0, 0.0))
		var t2 = normal.cross(Vec3.new(0.0, 1.0, 0.0))
		var tangent = (t1.lengthSqr > t2.lengthSqr) ? t1 : t2

    _positions.add(posA)
    _positions.add(posB)
    _positions.add(posC)
    _positions.add(posD)
    _normals.add(normal)
    _normals.add(normal)
    _normals.add(normal)
    _normals.add(normal)
    _tangents.add(tangent)
    _tangents.add(tangent)
    _tangents.add(tangent)
    _tangents.add(tangent)
    _uvs.add(uvA)
    _uvs.add(uvB)
    _uvs.add(uvC)
    _uvs.add(uvD)
    _indices.add(_baseIndex + 0)
    _indices.add(_baseIndex + 1)
    _indices.add(_baseIndex + 2)
    _indices.add(_baseIndex + 2)
    _indices.add(_baseIndex + 3)
    _indices.add(_baseIndex + 0)
    _baseIndex = _baseIndex + 4
  }

  make {
    var mesh = Mesh.create()
    mesh.positions = _positions
    mesh.normals   = _normals
    mesh.tangents  = _tangents
    mesh.texCoords = _uvs
    mesh.indices   = _indices
    return mesh
  }
}

class City {
  construct new(numBlocks, blockSize, streetSize, minHeight, maxHeight) {
    _numBlocks     = numBlocks
    _blockSize     = blockSize
    _streetSize    = streetSize
    _minHeight     = minHeight
    _maxHeight     = maxHeight

    _meshBuildings = CustomMesh.new()
    _meshRoads     = CustomMesh.new()
    _meshGreenery  = CustomMesh.new()

    constructBlocks()

    makeBuildings()
    makeRoads()
    makeGreenery()
  }

  makeBuildings() {
    _buildings = GameObject.new()
    _buildings.name = "buildings"
    
    var meshRender = _buildings.addComponent(MeshRender)
    meshRender.mesh    = _meshBuildings.make
    meshRender.subMesh = 0
    meshRender.albedo  = Texture.load("resources/textures/facade/Facade01_alb.jpg")
    meshRender.normal  = Texture.load("resources/textures/facade/Facade01_nrm.jpg")
    meshRender.DMRA    = Texture.load("resources/textures/facade/Facade01_dmra.png")
    meshRender.makeStatic()

    var collider = _buildings.addComponent(Collider)
    collider.makeMeshCollider(meshRender.mesh, 0)
    collider.layers = PhysicsLayers.General
  }

  makeRoads() {
    _roads = GameObject.new()
    _roads.name = "roads"
    
    var meshRender = _roads.addComponent(MeshRender)
    meshRender.mesh    = _meshRoads.make
    meshRender.subMesh = 0
    meshRender.albedo  = Texture.load("resources/textures/asphalt/Asphalt10_alb.jpg")
    meshRender.normal  = Texture.load("resources/textures/asphalt/Asphalt10_nrm.jpg")
    meshRender.DMRA    = Texture.load("resources/textures/asphalt/Asphalt10_dmra.png")
    meshRender.makeStatic()

    var collider = _roads.addComponent(Collider)
    collider.makeMeshCollider(meshRender.mesh, 0)
    collider.layers = PhysicsLayers.General
  }

  makeGreenery() {
    _greenery = GameObject.new()
    _greenery.name = "greenery"
    
    var meshRender = _greenery.addComponent(MeshRender)
    meshRender.mesh    = _meshGreenery.make
    meshRender.subMesh = 0
    meshRender.albedo  = Texture.load("resources/textures/ground/grass1_alb.png")
    meshRender.normal  = Texture.load("resources/textures/ground/grass1_nrm.png")
    meshRender.DMRA    = Texture.load("resources/textures/ground/grass1_dmra.png")
    meshRender.makeStatic()

    var collider = _greenery.addComponent(Collider)
    collider.makeMeshCollider(meshRender.mesh, 0)
    collider.layers = PhysicsLayers.General
  }

  constructSquare(min, max) {
    var size = (max - min) * 0.5
    var pos  = (max + min) * 0.5

    _meshGreenery.addQuad(pos + Vec3.new(-size.x,  size.y, -size.z), pos + Vec3.new(-size.x,  size.y,  size.z), pos + Vec3.new( size.x,  size.y,  size.z), pos + Vec3.new( size.x,  size.y, -size.z), Vec2.new(0.0, size.z), Vec2.new(0.0, 0.0), Vec2.new(size.x, 0.0), Vec2.new(size.x, size.z))
  }

  constructRoad(min, max) {
    var size = (max - min) * 0.5
    var pos  = (max + min) * 0.5

    _meshRoads.addQuad(pos + Vec3.new(-size.x,  size.y, -size.z), pos + Vec3.new(-size.x,  size.y,  size.z), pos + Vec3.new( size.x,  size.y,  size.z), pos + Vec3.new( size.x,  size.y, -size.z), Vec2.new(0.0, size.z), Vec2.new(0.0, 0.0), Vec2.new(size.x, 0.0), Vec2.new(size.x, size.z))
  }

  spawnBlock(fullMin, fullMax, num, scale) {
    var advantage = Fn.new { |min, max| Math.max(Math.random(min, max), Math.random(min, max)) }
    var convert   = Fn.new { |y, x, w| y * w + x }

    var heights = List.filled(num * num, 10)
    heights[convert.call(1, 1, num)] = (advantage.call(_minHeight, _maxHeight) / scale).round * scale
    
    heights[convert.call(0, 1, num)] = (advantage.call(_minHeight, heights[convert.call(1, 1, num)]) / scale).round * scale
    heights[convert.call(1, 0, num)] = (advantage.call(_minHeight, heights[convert.call(1, 1, num)]) / scale).round * scale
    heights[convert.call(2, 1, num)] = (advantage.call(_minHeight, heights[convert.call(1, 1, num)]) / scale).round * scale
    heights[convert.call(1, 2, num)] = (advantage.call(_minHeight, heights[convert.call(1, 1, num)]) / scale).round * scale

    heights[convert.call(0, 0, num)] = (advantage.call(_minHeight, Math.min(heights[convert.call(1, 0, num)], heights[convert.call(0, 1, num)])) / scale).round * scale
    heights[convert.call(2, 0, num)] = (advantage.call(_minHeight, Math.min(heights[convert.call(1, 0, num)], heights[convert.call(2, 1, num)])) / scale).round * scale
    heights[convert.call(0, 2, num)] = (advantage.call(_minHeight, Math.min(heights[convert.call(1, 2, num)], heights[convert.call(0, 1, num)])) / scale).round * scale
    heights[convert.call(2, 2, num)] = (advantage.call(_minHeight, Math.min(heights[convert.call(1, 2, num)], heights[convert.call(2, 1, num)])) / scale).round * scale

    for (z2 in 0...num) {
      for (x2 in 0...num) {
        var height    = heights[convert.call(z2, x2, num)]
        var lowLeft   = x2 > 0       ? heights[convert.call(z2, x2 - 1, num)] : 0.0
        var lowRight  = x2 < num - 1 ? heights[convert.call(z2, x2 + 1, num)] : 0.0
        var lowTop    = z2 > 0       ? heights[convert.call(z2 - 1, x2, num)] : 0.0
        var lowBottom = z2 < num - 1 ? heights[convert.call(z2 + 1, x2, num)] : 0.0

        var min = Vec3.new(Math.lerp(fullMin.x, fullMax.x, (x2 + 0) / num), fullMin.y,          Math.lerp(fullMin.z, fullMax.z, (z2 + 0) / num))
        var max = Vec3.new(Math.lerp(fullMin.x, fullMax.x, (x2 + 1) / num), fullMin.y + height, Math.lerp(fullMin.z, fullMax.z, (z2 + 1) / num))

        var size = (max - min) * 0.5
        var pos  = (max + min) * 0.5
        var uv   = size * (1.0 / scale)

        if (lowTop    < height) _meshBuildings.addQuad(pos + Vec3.new(-size.x, -size.y, -size.z), pos + Vec3.new(-size.x,  size.y, -size.z), pos + Vec3.new( size.x,  size.y, -size.z), pos + Vec3.new( size.x, -size.y, -size.z), Vec2.new(0.0,  uv.y), Vec2.new(0.0, 0.0),  Vec2.new(uv.x, 0.0), Vec2.new(uv.x, uv.y))
        if (lowBottom < height) _meshBuildings.addQuad(pos + Vec3.new(-size.x, -size.y,  size.z), pos + Vec3.new( size.x, -size.y,  size.z), pos + Vec3.new( size.x,  size.y,  size.z), pos + Vec3.new(-size.x,  size.y,  size.z), Vec2.new(uv.x, uv.y), Vec2.new(0.0, uv.y), Vec2.new(0.0,  0.0), Vec2.new(uv.x, 0.0))
        if (lowLeft   < height) _meshBuildings.addQuad(pos + Vec3.new(-size.x, -size.y,  size.z), pos + Vec3.new(-size.x,  size.y,  size.z), pos + Vec3.new(-size.x,  size.y, -size.z), pos + Vec3.new(-size.x, -size.y, -size.z), Vec2.new(0.0,  uv.y), Vec2.new(0.0, 0.0),  Vec2.new(uv.x, 0.0), Vec2.new(uv.x, uv.y))
        if (lowRight  < height) _meshBuildings.addQuad(pos + Vec3.new( size.x, -size.y, -size.z), pos + Vec3.new( size.x,  size.y, -size.z), pos + Vec3.new( size.x,  size.y,  size.z), pos + Vec3.new( size.x, -size.y,  size.z), Vec2.new(0.0,  uv.y), Vec2.new(0.0, 0.0),  Vec2.new(uv.x, 0.0), Vec2.new(uv.x, uv.y))
        
        // Top.
        _meshBuildings.addQuad(pos + Vec3.new(-size.x,  size.y, -size.z), pos + Vec3.new(-size.x,  size.y,  size.z), pos + Vec3.new( size.x,  size.y,  size.z), pos + Vec3.new( size.x,  size.y, -size.z), Vec2.new(0.0,  uv.z), Vec2.new(0.0, 0.0),  Vec2.new(uv.x, 0.0), Vec2.new(uv.x, uv.z))
      }
    }
  }

  constructBlocks() {
    for (z in (-_numBlocks.y / 2)...(_numBlocks.y / 2)) {
      var zOffset = z * _blockSize.y
      for (x in (-_numBlocks.x / 2)...(_numBlocks.x / 2)) {

        var xOffset = x * _blockSize.x
        var size = Vec3.new(_blockSize.x - _streetSize.x, 0.0, _blockSize.y - _streetSize.y)
        var position = Vec3.new(xOffset, 0.0, zOffset)
        var fullMin = position - Vec3.new(size.x / 2, 0.0, size.z / 2)
        var fullMax = position + Vec3.new(size.x / 2, 0.0, size.z / 2)

        if (!(z == 0 && x == 0 || Math.random() > 0.8)) {
          var num = 3
          var scale = 10
          
          spawnBlock(fullMin, fullMax, num, scale)
          //spawnBlock(fullMin + Vec3.new(size.x * 0.0, 0, size.z * 0.0), fullMin + Vec3.new(size.x * 0.5, 0, size.z * 0.5), num, scale)
          //spawnBlock(fullMin + Vec3.new(size.x * 0.5, 0, size.z * 0.0), fullMin + Vec3.new(size.x * 1.0, 0, size.z * 0.5), num, scale)
          //spawnBlock(fullMin + Vec3.new(size.x * 0.0, 0, size.z * 0.5), fullMin + Vec3.new(size.x * 0.5, 0, size.z * 1.0), num, scale)
          //spawnBlock(fullMin + Vec3.new(size.x * 0.5, 0, size.z * 0.5), fullMin + Vec3.new(size.x * 1.0, 0, size.z * 1.0), num, scale)
        } else {
          constructSquare(fullMin, fullMax)
        }
        constructRoad(Vec3.new(fullMin.x,                 fullMin.y, fullMin.z - _streetSize.y), Vec3.new(fullMax.x, fullMin.y, fullMin.z))
        constructRoad(Vec3.new(fullMin.x - _streetSize.x, fullMin.y, fullMin.z),                 Vec3.new(fullMin.x, fullMin.y, fullMax.z))
        constructRoad(Vec3.new(fullMin.x - _streetSize.x, fullMin.y, fullMin.z - _streetSize.y), Vec3.new(fullMin.x, fullMin.y, fullMin.z))
      }
    }
  }
}

class World {
    construct new() {
    }

    constructNodes() {
      _nodeMap = NodeMap.new()
      _toggleNodeEditor = false
      var nodeEditor = NodeEditor.new(_nodeMap)
      nodeEditor.deserialize()
    }

    getNextNode() {
      if (_positionList == null || _positionList.isEmpty) {
        return
      }

      _currPosition = _positionList[0]
      _positionList.removeAt(0)
    }

    moveMoveMe() {
      if (_moveMe == null) {
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
      _city = City.new(Vec2.new(15), Vec2.new(100), Vec2.new(25), 50, 100)
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
      _currNode = null

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

    initialize() {
      _post_processer = PostProcessor.new()
      _lighting = Lighting.new()

      _camera = GameObject.new()
      _camera.addComponent(FreeLookCamera)
      
      initializeCity()
      //initializeRoom()

      GUI.loadURL("file:///resources/web-pages/menu.html")
      GUI.bindCallback("changedSetting(_,_)", this)
      GUI.bindCallback("movedToMenu(_)", this)
    }
    deinitialize() {

    }
    update() {
      GUI.executeJavaScript("updateDebug(%(Time.deltaTime));")
      
      // Update the RSM light.
      var sin_y_rot = -Math.sin(Math.deg2Rad * _camera.getComponent(FreeLookCamera).rotation.y)
      var cos_y_rot = -Math.cos(Math.deg2Rad * _camera.getComponent(FreeLookCamera).rotation.y)
      var offset = Vec3.new(sin_y_rot, 0.0, cos_y_rot).normalized * 12.5
      var transform = _lighting.rsm.getComponent(Transform)
      transform.worldPosition = _camera.transform.worldPosition + offset

      var toggleNodeEditor = InputController.ToggleNodeEditor != 0
      var toggleEditor = toggleNodeEditor && !_toggleNodeEditor
      _toggleNodeEditor = toggleNodeEditor

      if (toggleEditor) {
        if (_nodeEditor == null) {
          _nodeEditor = NodeEditor.new(_nodeMap)
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
      _camera.getComponent(FreeLookCamera).hasInput = menu == "" ? true : false
    }
}
