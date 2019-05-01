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

import "resources/scripts/wren/post_processor" for PostProcessor
import "resources/scripts/wren/input_controller" for InputController
import "resources/scripts/wren/camera" for FreeLookCamera
import "resources/scripts/wren/lighting" for Lighting
import "resources/scripts/wren/trees" for Trees
import "resources/scripts/wren/item_manager" for ItemManager
import "resources/scripts/wren/door" for Door
import "resources/scripts/wren/physics_layers" for PhysicsLayers

class PositionNode {
  construct new(position) {
    _position = position
  }
  position { _position }
}

class Node {
  getDir(a, b, s) {
      var c = (b - a)
      c.y = 0.0
      var d = c.x
      c.x = -c.z
      c.z = d
      return c.normalized * s
  }
  drawCircle(center, radius, segments, colour) {
    for (i in 0..segments) {
      var i1 = i % segments
      var i2 = (i + 1) % segments

      var s1 = Math.sin((i1 / segments) * Math.tau)
      var c1 = Math.cos((i1 / segments) * Math.tau)
      var s2 = Math.sin((i2 / segments) * Math.tau)
      var c2 = Math.cos((i2 / segments) * Math.tau)

      Debug.drawLine(center + Vec3.new(s1 * radius, 0.0, c1 * radius), center + Vec3.new(s2 * radius, 0.0, c2 * radius), colour)
    }
  }

  valid { true }

  construct new(index, position) {
    _index       = index
    _position    = position
    _connections = []
    _drawIndex   = 0
    _searchValue = -1
  }

  addConnection(connection) {
    for (c in _connections) {
      if (c == connection) {
        return
      }
    }

    _connections.add(connection)
    connection.addConnection(this)
    return this
  }

  evaluateRoute(searchValue) {
    if (searchValue > _searchValue && _searchValue != -1) {
      return
    }

    _searchValue = searchValue
    
    for (connection in connections) {
      var distance = (position - connection.position).length
      connection.evaluateRoute(searchValue + distance)
    }
  }

  getNextNode() {
    var bestValue = -1
    var bestNode = null
    
    for (connection in connections) {
      if (connection.getSearchValue < bestValue || bestValue == -1) {
        bestValue = connection.getSearchValue
        bestNode = connection
      }
    }

    return bestNode
  }

  getSearchValue { _searchValue }
  index { _index }

  reset() {
    if (_searchValue == -1) {
      return
    }

    _searchValue = -1
    
    for (connection in connections) {
      connection.reset()
    }
  }

  connections { _connections }
  position    { _position }

  validDrawIndex(drawIndex) { _drawIndex < drawIndex }

  render(drawIndex) {
    if (!validDrawIndex(drawIndex)) {
      return
    }
    _drawIndex = drawIndex

    var scale = 0.5
    var segments = 12
    var colourLine = Vec4.new(1.0, 0.0, 0.0, 1.0)
    var colourCircle = Vec4.new(0.0, 0.0, 1.0, 1.0)
    drawCircle(position, scale, segments, colourCircle)

    for (connection in connections) {
      var dir = getDir(connection.position, position, scale)
    
      // Side lines.
      Debug.drawLine(position + dir, connection.position + dir, colourLine)
      Debug.drawLine(position - dir, connection.position - dir, colourLine)

      connection.render(drawIndex)
    }
    return this
  }
}

class World {
    construct new() { _drawIndex = 0 }
    
    constructNodes() {
      /*
      Ground floor
      _______________
      |3  4 | | 9  8|
      | |   | |   | |
      | | 5  11 10| |
      | |         | |
      |1 17  0  19 6|
      | |         | |
      | |         | |
      |2|18__21_20|7|

      First floor
      _______________
      |13___ 12___15|
      | |   | |   | |
      | |    11   | |
      |14         | |
      |_          | |
      | |         | |
      | |         | |
      |_|_________16|
      */

      // Ground floor
      _nodeList = []
      _nodes = List.filled(22, null)
      _nodes[0]  = Node.new(0, Vec3.new(  0,  0,   0))
      _nodes[1]  = Node.new(1, Vec3.new(  0,  0, -10))
      _nodes[2]  = Node.new(2, Vec3.new(-11,  0, -10))
      _nodes[3]  = Node.new(3, Vec3.new( 10,  0, -10))
      _nodes[4]  = Node.new(4, Vec3.new( 10,  0,  -6))
      _nodes[5]  = Node.new(5, Vec3.new(  6,  0,  -6))
      _nodes[6]  = Node.new(6, Vec3.new(  0,  0,  10))
      _nodes[7]  = Node.new(7, Vec3.new(-11,  0,  10))
      _nodes[8]  = Node.new(8, Vec3.new( 10,  0,  10))
      _nodes[9]  = Node.new(9, Vec3.new( 10,  0,   6))
      _nodes[10] = Node.new(10, Vec3.new(  6,  0,   6))
      _nodes[11] = Node.new(11, Vec3.new(  6,  0,   0))
      _nodes[17] = Node.new(17, Vec3.new(  0,  0,  -6))
      _nodes[18] = Node.new(18, Vec3.new(-11,  0,  -6))
      _nodes[19] = Node.new(19, Vec3.new(  0,  0,   6))
      _nodes[20] = Node.new(20, Vec3.new(-11,  0,   6))
      _nodes[21] = Node.new(21, Vec3.new(-11,  0,   0))
      // First floor
      _nodes[12] = Node.new(12, Vec3.new( 10,  1,   0))
      _nodes[13] = Node.new(13, Vec3.new( 10,  5, -10))
      _nodes[14] = Node.new(14, Vec3.new(  4,  5, -10))
      _nodes[15] = Node.new(15, Vec3.new( 10,  5,  10))
      _nodes[16] = Node.new(16, Vec3.new(-11,  5,  10))

      _nodes[0].addConnection(_nodes[17])
      _nodes[0].addConnection(_nodes[5])
      _nodes[0].addConnection(_nodes[19])
      _nodes[0].addConnection(_nodes[10])
      _nodes[0].addConnection(_nodes[11])

      _nodes[1].addConnection(_nodes[17])
      _nodes[1].addConnection(_nodes[2])
      _nodes[1].addConnection(_nodes[3])
      
      _nodes[2].addConnection(_nodes[1])

      _nodes[3].addConnection(_nodes[1])
      _nodes[3].addConnection(_nodes[4])
      
      _nodes[4].addConnection(_nodes[3])
      _nodes[4].addConnection(_nodes[5])

      _nodes[5].addConnection(_nodes[0])
      _nodes[5].addConnection(_nodes[4])
      _nodes[5].addConnection(_nodes[11])
      _nodes[5].addConnection(_nodes[17])

      _nodes[6].addConnection(_nodes[19])
      _nodes[6].addConnection(_nodes[7])
      _nodes[6].addConnection(_nodes[8])

      _nodes[7].addConnection(_nodes[6])

      _nodes[8].addConnection(_nodes[6])
      _nodes[8].addConnection(_nodes[9])

      _nodes[9].addConnection(_nodes[8])
      _nodes[9].addConnection(_nodes[10])
 
      _nodes[10].addConnection(_nodes[0])
      _nodes[10].addConnection(_nodes[9])
      _nodes[10].addConnection(_nodes[11])
      _nodes[10].addConnection(_nodes[19])

      _nodes[11].addConnection(_nodes[0])
      _nodes[11].addConnection(_nodes[5])
      _nodes[11].addConnection(_nodes[10])
      _nodes[11].addConnection(_nodes[12])
      _nodes[11].addConnection(_nodes[17])
      _nodes[11].addConnection(_nodes[19])

      _nodes[12].addConnection(_nodes[11])
      _nodes[12].addConnection(_nodes[13])
      _nodes[12].addConnection(_nodes[15])

      _nodes[13].addConnection(_nodes[12])
      _nodes[13].addConnection(_nodes[14])

      _nodes[15].addConnection(_nodes[12])
      _nodes[15].addConnection(_nodes[16])

      _nodes[16].addConnection(_nodes[15])

      _nodes[17].addConnection(_nodes[0])
      _nodes[17].addConnection(_nodes[1])
      _nodes[17].addConnection(_nodes[5])
      _nodes[17].addConnection(_nodes[11])
      _nodes[17].addConnection(_nodes[18])
      _nodes[17].addConnection(_nodes[21])
      
      _nodes[18].addConnection(_nodes[0])
      _nodes[18].addConnection(_nodes[17])
      _nodes[18].addConnection(_nodes[21])

      _nodes[19].addConnection(_nodes[0])
      _nodes[19].addConnection(_nodes[6])
      _nodes[19].addConnection(_nodes[10])
      _nodes[19].addConnection(_nodes[11])
      _nodes[19].addConnection(_nodes[20])
      _nodes[19].addConnection(_nodes[21])
      
      _nodes[20].addConnection(_nodes[0])
      _nodes[20].addConnection(_nodes[19])
      _nodes[20].addConnection(_nodes[21])

      _nodes[21].addConnection(_nodes[0])
      _nodes[21].addConnection(_nodes[17])
      _nodes[21].addConnection(_nodes[18])
      _nodes[21].addConnection(_nodes[19])
      _nodes[21].addConnection(_nodes[20])
    }

    findClosestNode(position) {
      var closest = -1
      var closestLength = -1

      for (node in _nodes) {
        var length = (position - node.position).lengthSqr
        if (length < closestLength || closestLength == -1) {
          closest = node.index
          closestLength = length
        }
      }

      return closest
    }

    getNodeList(nodeToMoveTo, nodeToComeFrom) {
      _nodeList = []

      _nodes[nodeToMoveTo].evaluateRoute(0)

      var currNode = _nodes[nodeToComeFrom]
      while (currNode.getSearchValue != 0) {
        currNode = currNode.getNextNode()
        _nodeList.insert(-1, currNode)
      }

      _nodes[nodeToMoveTo].reset()
    }
    
    renderNodes() {
      _drawIndex = _drawIndex + 1
      _nodes[0].render(_drawIndex)
    }

    getNextNode() {
      if (_nodeList.isEmpty) {
        return
      }

      _currNode = _nodeList[0]
      _nodeList.removeAt(0)
    }

    moveMoveMe() {
      if (_currNode == null && _nodeList.count <= 0) {
        getNodeList(findClosestNode(_camera.transform.worldPosition), findClosestNode(_moveMe.transform.worldPosition))
        return
      }

      if (_currNode == null) {
        getNextNode()
      }

      var offset = _currNode.position - _moveMe.transform.worldPosition
      offset.y = 0.0
      var length = offset.length
      var movement = 2.5 * Time.fixedDeltaTime
      if (length < movement) {
        movement = length
      }

      if (length < 0.1) {
        var currNode = _currNode
        getNodeList(findClosestNode(_camera.transform.worldPosition), findClosestNode(_moveMe.transform.worldPosition))
        getNextNode()
        if (_currNode == currNode) {
          getNextNode()
        }
        //_currNode = _currNode.connections[Math.random(_currNode.connections.count - 1).round]
      }

      offset = offset.normalized * movement
      _moveMe.transform.worldPosition = _moveMe.transform.worldPosition + offset
    }

    initialize() {
      constructNodes()

      _item_manager = GameObject.new().addComponent(ItemManager)

      _post_processer = PostProcessor.new()
      _lighting = Lighting.new()

      _camera = GameObject.new()
      _camera.addComponent(FreeLookCamera)
      
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

      GUI.loadURL("file:///resources/web-pages/menu.html")
      GUI.bindCallback("changedSetting(_,_)", this)
      GUI.bindCallback("movedToMenu(_)", this)
    }
    deinitialize() {

    }
    update() {
      GUI.executeJavaScript("updateDebug(%(Time.deltaTime));")
      
      // Update the RSM light.
      //var sin_y_rot = -Math.sin(Math.deg2Rad * _camera.getComponent(FreeLookCamera).rotation.y)
      //var cos_y_rot = -Math.cos(Math.deg2Rad * _camera.getComponent(FreeLookCamera).rotation.y)
      //var offset = Vec3.new(sin_y_rot, 0.0, cos_y_rot).normalized * 12.5
      //var transform = _lighting.rsm.getComponent(Transform)
      //transform.worldPosition = _camera.transform.worldPosition + offset

      renderNodes()
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
