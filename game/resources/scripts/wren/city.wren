import "Core" for Vec2, Vec3, Vec4
import "Core" for Texture, Shader, Mesh
import "Core" for GameObject, Transform, Camera, MeshRender, Lod, RigidBody, WaveSource, Collider
import "Core" for Input, Keys, Buttons, Axes
import "Core" for Math
import "Core" for Graphics, GUI, Time, File, Assert
import "Core" for MonoBehaviour
import "Core" for PostProcess, Console, Physics, Debug, Sort
import "Core" for NavMesh

import "resources/scripts/wren/physics_layers" for PhysicsLayers
//import "resources/scripts/wren/node_map" for Node, NodeMap, NodeEditor

class CustomMesh {
  construct new () {
    _mesh = Mesh.create()
    clear()
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

  clear() {
      _positions = []
      _normals   = []
      _tangents  = []
      _uvs       = []
      _indices   = []
      _baseIndex = 0
  }

  make {
    _mesh.positions = _positions
    _mesh.normals   = _normals
    _mesh.tangents  = _tangents
    _mesh.texCoords = _uvs
    _mesh.indices   = _indices
    return _mesh
  }
}

class City {
  construct new(numBlocks, blockSize, streetSize, sidewalkSize, minHeight, maxHeight) {
    _numBlocks     = numBlocks
    _blockSize     = blockSize
    _streetSize    = streetSize
    _sidewalkSize  = sidewalkSize
    _minHeight     = minHeight
    _maxHeight     = maxHeight

    _meshBuildings = CustomMesh.new()
    _meshRoads     = CustomMesh.new()
    _meshSidewalks = CustomMesh.new()
    _meshGreenery  = CustomMesh.new()

    constructBlocks()
    constructNavMesh()

    makeBuildings()
    makeRoads()
    makeSidewalks()
    makeGreenery()
  }

  numBlocks    { _numBlocks    }
  blockSize    { _blockSize    }
  streetSize   { _streetSize   }
  sidewalkSize { _sidewalkSize }
  navMesh      { _navMesh      }

  draw() {
    _nodeMap.draw()
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

  makeSidewalks() {
    _sidewalks = GameObject.new()
    _sidewalks.name = "sidewalks"
    
    var meshRender = _sidewalks.addComponent(MeshRender)
    meshRender.mesh    = _meshSidewalks.make
    meshRender.subMesh = 0
    meshRender.albedo  = Texture.load("resources/textures/paving/PavingStones28_alb.jpg")
    meshRender.normal  = Texture.load("resources/textures/paving/PavingStones28_nrm.jpg")
    meshRender.DMRA    = Texture.load("resources/textures/paving/PavingStones28_dmra.png")
    meshRender.makeStatic()

    var collider = _sidewalks.addComponent(Collider)
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

  constructSidewalk(min, max) {
    var size = (max - min) * 0.5
    var pos  = (max + min) * 0.5
    _meshSidewalks.addQuad(pos + Vec3.new(-size.x,  size.y, -size.z), pos + Vec3.new(-size.x,  size.y,  size.z), pos + Vec3.new( size.x,  size.y,  size.z), pos + Vec3.new( size.x,  size.y, -size.z), Vec2.new(0.0, size.z), Vec2.new(0.0, 0.0), Vec2.new(size.x, 0.0), Vec2.new(size.x, size.z))
  }
  constructRoad(min, max) {
    var size = (max - min) * 0.5
    var pos  = (max + min) * 0.5
    _meshRoads.addQuad(pos + Vec3.new(-size.x,  size.y, -size.z), pos + Vec3.new(-size.x,  size.y,  size.z), pos + Vec3.new( size.x,  size.y,  size.z), pos + Vec3.new( size.x,  size.y, -size.z), Vec2.new(0.0, size.z), Vec2.new(0.0, 0.0), Vec2.new(size.x, 0.0), Vec2.new(size.x, size.z))
  }
  constructRoad(min, max, type) {
    // 0: up.
    // 1: right.
    // 2: crossroad.
    if (type == 0) {
      constructSidewalk(Vec3.new(min.x, min.y, min.z), Vec3.new(min.x + _sidewalkSize.x / 2, max.y, max.z))
      constructSidewalk(Vec3.new(max.x - _sidewalkSize.x / 2, min.y, min.z), Vec3.new(max.x, max.y, max.z))
      constructRoad(Vec3.new(min.x + _sidewalkSize.x / 2, min.y, min.z), Vec3.new(max.x - _sidewalkSize.x / 2, max.y, max.z))
    }
    if (type == 1) {
      constructSidewalk(Vec3.new(min.x, min.y, min.z), Vec3.new(max.x, max.y, min.z + _sidewalkSize.y / 2))
      constructSidewalk(Vec3.new(min.x, min.y, max.z - _sidewalkSize.y / 2), Vec3.new(max.x, max.y, max.z))
      constructRoad(Vec3.new(min.x, min.y, min.z + _sidewalkSize.y / 2), Vec3.new(max.x, max.y, max.z - _sidewalkSize.y / 2))
    }
    if (type == 2) {
      constructRoad(Vec3.new(min.x, min.y, min.z), Vec3.new(max.x, max.y, min.z + _sidewalkSize.y / 2), 0)
      constructRoad(Vec3.new(min.x, min.y, max.z - _sidewalkSize.y / 2), Vec3.new(max.x, max.y, max.z), 0)
      constructRoad(Vec3.new(min.x, min.y, min.z + _sidewalkSize.y / 2), Vec3.new(max.x, max.y, max.z - _sidewalkSize.y / 2))
    }
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
        var min = position - Vec3.new(size.x / 2, 0.0, size.z / 2)
        var max = position + Vec3.new(size.x / 2, 0.0, size.z / 2)

        if (!(z == 0 && x == 0 || Math.random() > 0.8)) {
          var num = 3
          var scale = 10
          
          spawnBlock(min, max, num, scale)
        } else {
          constructSquare(min, max)
        }
        
        // _____________1____2
        //|max          |    |
        //|             |    |
        //|    BLOCK    | RD |
        //|             |    |
        //|             |    |
        //3__________min4____5
        //|     RD      |    |
        //6_____________7____8
        // str = min - streetSize
        // 1: x: min.x
        //    y: max.y
        // 3: x: max.x
        //    y: min.y
        // 4: x: min.x
        //    y: min.y
        // 5: x: str.x
        //    y: min.y
        // 7: x: min.x
        //    y: str.y
        // 8: x: str.x
        //    y: str.y

        var str = min - Vec3.new(_streetSize.x, 0, _streetSize.y)
        var p1 = Vec3.new(min.x, min.y, max.z)
        var p3 = Vec3.new(max.x, min.y, min.z)
        var p4 = Vec3.new(min.x, min.y, min.z)
        var p5 = Vec3.new(str.x, min.y, min.z)
        var p7 = Vec3.new(min.x, min.y, str.z)
        var p8 = Vec3.new(str.x, min.y, str.z)

        constructRoad(p5, p1, 0)
        constructRoad(p7, p3, 1)
        constructRoad(p8, p4, 2)
      }
    }
  }

  constructNavMesh() {
    _navMesh = NavMesh.new()
    
    var nodes = List.filled(_numBlocks.y, null)
    for (z in 0..._numBlocks.y) {
      nodes[z] = List.filled(_numBlocks.x, null)
    }
    
    for (z in (-_numBlocks.y / 2)...(_numBlocks.y / 2)) {
      var zOffset = z * _blockSize.y - (_blockSize.y * 0.5)
      for (x in (-_numBlocks.x / 2)...(_numBlocks.x / 2)) {
        var xOffset = x * _blockSize.x - (_blockSize.x * 0.5)

        var node = _navMesh.addNode(Vec3.new(xOffset, 0.0, zOffset))
        if (x > -_numBlocks.x / 2) node.addConnection(nodes[z + _numBlocks.y / 2][x + _numBlocks.x / 2 - 1])
        if (z > -_numBlocks.y / 2) node.addConnection(nodes[z + _numBlocks.y / 2 - 1][x + _numBlocks.x / 2])
        nodes[z + _numBlocks.y / 2][x + _numBlocks.x / 2] = node
      }
    }
  }
}