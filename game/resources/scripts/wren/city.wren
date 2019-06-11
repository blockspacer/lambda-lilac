import "Core" for Vec2, Vec3, Vec4
import "Core" for Texture, Shader, Mesh, TextureFormat
import "Core" for GameObject, Transform, Camera, MeshRender, Lod, RigidBody, WaveSource, Collider
import "Core" for Input, Keys, Buttons, Axes
import "Core" for Math
import "Core" for Graphics, GUI, Time, File, Assert
import "Core" for MonoBehaviour, Light, LightTypes
import "Core" for PostProcess, Console, Physics, Debug, Sort
import "Core" for NavMesh, TriNavMesh

import "resources/scripts/wren/physics_layers" for PhysicsLayers
//import "resources/scripts/wren/node_map" for Node, NodeMap, NodeEditor
import "resources/scripts/wren/meshes" for MeshCreator

class Spline {
  construct new(a, b, c, d) {
    _a = a
    _b = b
    _c = c
    _d = d
  }
  
	length(precision) {
		var length = 0.0
		var b
		for (i in 0...precision) {
			var a = point(i / precision)
			if (i == 0) b = a
			var c = a - b
			length = length + Math.sqrt(c.x * c.x + c.y * c.y)
			b = a
		}
		return length
	}

	point(val) {
		return _a * val * val * val + _b * val * val + _c * val + _d
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
    _actorSize     = 1.0

    _lightPositions = []

    _meshBuildings = MeshCreator.new()
    _meshRoads     = MeshCreator.new()
    _meshSidewalks = MeshCreator.new()
    _meshGreenery  = MeshCreator.new()
    _meshLanterns  = MeshCreator.new()
    _meshLights    = MeshCreator.new()

    constructTriNavMesh()
    constructBlocks()
    constructNavMesh()
    constructLights()

    makeBuildings()
    makeRoads()
    makeSidewalks()
    makeGreenery()
    makeLanterns()
    makeLights()
    spawnLights()

    makeRoad()
  }

  numBlocks    { _numBlocks    }
  blockSize    { _blockSize    }
  streetSize   { _streetSize   }
  sidewalkSize { _sidewalkSize }
  navMesh      { _triNavMesh   }

  draw() {
    _nodeMap.draw()
  }

  getControlPoint(i) {
		while (i < 0) i = i + _controlPoints.count
		return _controlPoints[i.round % _controlPoints.count]
  }

  makeRoad() {
    var segments = 1024
    var width = 2.5
    
    _controlPoints = [ Vec3.new(-10.0, 0.1, -10.0), Vec3.new(-10.0, 0.1, 10.0), Vec3.new(10.0, 0.1, 10.0), Vec3.new(10.0, 0.1, -10.0) ]

    if (_controlPoints.count < 2) return

		var a
    var b
    var c
    var d
		var t = 0.5

		var splines = []

		// Draw intermediate segments
		for (j in 0..._controlPoints.count) {
			a = getControlPoint(j) * -t      + getControlPoint(j + 1) * (2.0 - t) + getControlPoint(j + 3) * t               + getControlPoint(j + 2) * (t - 2.0)
			b = getControlPoint(j) * 2.0 * t + getControlPoint(j + 1) * (t - 3.0) + getControlPoint(j + 2) * (3.0 - 2.0 * t) - getControlPoint(j + 3) * t
			c = getControlPoint(j) * -t      + getControlPoint(j + 2) * t
			d = getControlPoint(j + 1)
      splines.add(Spline.new(a, b, c, d))
		}

		var totalLength = 0.0
    var lengths = []
		for (spline in splines) {
      var length = spline.length(segments)
      totalLength = totalLength + length
      lengths.add(length)
    }

    var numSegmentsUsed = 0
    var points = []
		for (i in 0...splines.count) {
      var spline = splines[i]
			var numSegments = (segments * (lengths[i] / totalLength)).round
      numSegmentsUsed = numSegmentsUsed + numSegments
			for (k in 0...numSegments) points.add(spline.point(k / numSegments))
		}

    var mesh = MeshCreator.new()

    for (i in 0...points.count) {
      var prev = points[((i == 0) ? (points.count - 1) : (i - 1)) % points.count]
      var curr = points[i % points.count]
      var next = points[(i + 1) % points.count]
      var dirP = (curr - prev)
      var dirC = (next - curr)
      var lenP = dirP.length
      var lenC = dirC.length
      
      var t = -dirP.z
      dirP.z = dirP.x / lenP * width
      dirP.x = t / lenP * width

      t = -dirC.z
      dirC.z = dirC.x / lenC * width
      dirC.x = t / lenC * width

      var uvA = Vec2.new(0.0)
      var uvB = Vec2.new(0.0)
      var uvC = Vec2.new(0.0)
      var uvD = Vec2.new(0.0)
      mesh.addQuad(curr + dirP, next + dirC, next - dirC, curr - dirP, uvA, uvB, uvC, uvD)
      mesh.addQuad(curr - dirP, next - dirC, next + dirC, curr + dirP, uvA, uvB, uvC, uvD)
    }

    
    var road = GameObject.new()
    road.name = "road"
    
    var meshRender = road.addComponent(MeshRender)
    meshRender.mesh    = mesh.make
    meshRender.subMesh = 0
    meshRender.albedo  = Texture.load("resources/textures/facade/Facade01_alb.jpg")
    meshRender.normal  = Texture.load("resources/textures/facade/Facade01_nrm.jpg")
    meshRender.DMRA    = Texture.load("resources/textures/facade/Facade01_dmra.png")
    meshRender.makeStatic()

    var collider = road.addComponent(Collider)
    collider.makeMeshCollider(meshRender.mesh, 0)
    collider.layers = PhysicsLayers.General
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

    _meshBuildings.clear()
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

    _meshRoads.clear()
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

    _meshSidewalks.clear()
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
  
    _meshGreenery.clear()
  }

  makeLanterns() {
    _lanterns = GameObject.new()
    _lanterns.name = "lanterns"
    
    var meshRender = _lanterns.addComponent(MeshRender)
    meshRender.mesh    = _meshLanterns.make
    meshRender.subMesh = 0
    meshRender.albedo  = Texture.load("resources/textures/metal/Metal19_alb.jpg")
    meshRender.normal  = Texture.load("resources/textures/metal/Metal19_nrm.jpg")
    meshRender.DMRA    = Texture.load("resources/textures/metal/Metal19_dmra.png")
    meshRender.makeStatic()

    var collider = _lanterns.addComponent(Collider)
    collider.makeMeshCollider(meshRender.mesh, 0)
    collider.layers = PhysicsLayers.General
  
    _meshLanterns.clear()
  }

  makeLights() {
    _lightsMesh = GameObject.new()
    _lightsMesh.name = "lanterns"
    
    var meshRender = _lightsMesh.addComponent(MeshRender)
    meshRender.mesh    = _meshLights.make
    meshRender.subMesh = 0
    meshRender.emissivenessFactor = Vec3.new(10.0, 10.0, 7.0)
    meshRender.makeStatic()

    var collider = _lightsMesh.addComponent(Collider)
    collider.makeMeshCollider(meshRender.mesh, 0)
    collider.layers = PhysicsLayers.General
  
    _meshLights.clear()
  }

  spawnLights() {
    _lights = []
    for (position in _lightPositions) {
      var go = GameObject.new()
      go.name = "light"
      go.transform.worldPosition = position

      var light            = go.addComponent(Light)
      light.type           = LightTypes.Point
      light.lightColour    = Vec3.new(1.0, 1.0, 0.75)
      light.lightIntensity = 20.0
      light.depth          = 15.0

      _lights.add(go)
    }
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
    
    if (size.x < size.z) {
      addTriNavMeshQuad(min + Vec3.new(_actorSize, 0.0, -_actorSize), max - Vec3.new(_actorSize, 0.0, -_actorSize))
    } else {
      if (size.x == size.z) {
        addTriNavMeshQuad(min + Vec3.new(_actorSize, 0.0, _actorSize), max - Vec3.new(_actorSize, 0.0, _actorSize))
      } else {
        addTriNavMeshQuad(min + Vec3.new(-_actorSize, 0.0, _actorSize), max - Vec3.new(-_actorSize, 0.0, _actorSize))
      }
    }
  }
  constructRoad(min, max) {
    var size = (max - min) * 0.5
    var pos  = (max + min) * 0.5
    _meshRoads.addQuad(pos + Vec3.new(-size.x,  size.y, -size.z), pos + Vec3.new(-size.x,  size.y,  size.z), pos + Vec3.new( size.x,  size.y,  size.z), pos + Vec3.new( size.x,  size.y, -size.z), Vec2.new(0.0, size.z), Vec2.new(0.0, 0.0), Vec2.new(size.x, 0.0), Vec2.new(size.x, size.z))
  }

  constructLantern(pos) {
    // Pole
    var min = pos + Vec3.new(-0.2, 0.0, -0.2)
    var max = pos + Vec3.new( 0.2, 7.5,  0.2)
    spawnCube(min, max, 1.0, _meshLanterns)

    // Light
    min = pos + Vec3.new(-0.1, 7.5, -0.1)
    max = pos + Vec3.new( 0.1, 8.0,  0.1)
    spawnCube(min, max, 1.0, _meshLights)

    // Cap
    min = pos + Vec3.new(-0.4, 8.0,  -0.4)
    max = pos + Vec3.new( 0.4, 8.25,  0.4)
    spawnCube(min, max, 1.0, _meshLanterns)

    //_lightPositions.add(pos + Vec3.new(0.0, 7.75, 0.0))
  }

  constructRoad(min, max, type, navMesh) {
    // 0: up.
    // 1: right.
    // 2: crossroad.
    if (type == 0) {
      constructSidewalk(Vec3.new(min.x, min.y, min.z), Vec3.new(min.x + _sidewalkSize.x / 2, max.y, max.z))
      constructSidewalk(Vec3.new(max.x - _sidewalkSize.x / 2, min.y, min.z), Vec3.new(max.x, max.y, max.z))

      constructRoad(Vec3.new(min.x + _sidewalkSize.x / 2, min.y, min.z), Vec3.new(max.x - _sidewalkSize.x / 2, max.y, max.z))
      if (navMesh) {
        var mi = Vec3.new(min.x + _sidewalkSize.x / 2, min.y, min.z)
        var ma = Vec3.new(max.x - _sidewalkSize.x / 2, max.y, max.z)
        if (ma.x - mi.x < ma.z - mi.z) {
          addTriNavMeshQuad(mi + Vec3.new(_actorSize, 0.0, -_actorSize), ma - Vec3.new(_actorSize, 0.0, -_actorSize))
        } else {
          addTriNavMeshQuad(mi + Vec3.new(-_actorSize, 0.0, _actorSize), ma - Vec3.new(-_actorSize, 0.0, _actorSize))
        }
      }
    }
    if (type == 1) {
      constructSidewalk(Vec3.new(min.x, min.y, min.z), Vec3.new(max.x, max.y, min.z + _sidewalkSize.y / 2))
      constructSidewalk(Vec3.new(min.x, min.y, max.z - _sidewalkSize.y / 2), Vec3.new(max.x, max.y, max.z))

      constructRoad(Vec3.new(min.x, min.y, min.z + _sidewalkSize.y / 2), Vec3.new(max.x, max.y, max.z - _sidewalkSize.y / 2))
      if (navMesh) {
        var mi = Vec3.new(min.x, min.y, min.z + _sidewalkSize.y / 2)
        var ma = Vec3.new(max.x, max.y, max.z - _sidewalkSize.y / 2)
        if (ma.x - mi.x < ma.z - mi.z) {
          addTriNavMeshQuad(mi + Vec3.new(_actorSize, 0.0, -_actorSize), ma - Vec3.new(_actorSize, 0.0, -_actorSize))
        } else {
          addTriNavMeshQuad(mi + Vec3.new(-_actorSize, 0.0, _actorSize), ma - Vec3.new(-_actorSize, 0.0, _actorSize))
        }
      }
    }
    if (type == 2) {
      constructRoad(Vec3.new(min.x, min.y, min.z), Vec3.new(max.x, max.y, min.z + _sidewalkSize.y / 2), 0, true)
      constructRoad(Vec3.new(min.x, min.y, max.z - _sidewalkSize.y / 2), Vec3.new(max.x, max.y, max.z), 0, true)

      constructRoad(Vec3.new(min.x, min.y, min.z), Vec3.new(min.x + _sidewalkSize.x / 2, max.y, max.z), 1, true)
      constructRoad(Vec3.new(max.x - _sidewalkSize.x / 2, min.y, min.z), Vec3.new(max.x, max.y, max.z), 1, true)

      constructRoad(Vec3.new(min.x + _sidewalkSize.x / 2, min.y, min.z + _sidewalkSize.y / 2), Vec3.new(max.x - _sidewalkSize.x / 2, max.y, max.z - _sidewalkSize.y / 2))
    }
  }
  constructRoad(min, max, type) {
    constructRoad(min, max, type, false)
  }

  spawnCube(min, max, scale, mesh) {
    var size = (max - min) * 0.5
    var pos  = (max + min) * 0.5
    var uv   = size * (1.0 / scale)
    /*-z*/mesh.addQuad(pos + Vec3.new(-size.x, -size.y, -size.z), pos + Vec3.new(-size.x,  size.y, -size.z), pos + Vec3.new( size.x,  size.y, -size.z), pos + Vec3.new( size.x, -size.y, -size.z), Vec2.new(0.0,  0.0), Vec2.new(0.0,  uv.y), Vec2.new(uv.x, uv.y), Vec2.new(uv.x, 0.0))
    /*+z*/mesh.addQuad(pos + Vec3.new(-size.x, -size.y,  size.z), pos + Vec3.new( size.x, -size.y,  size.z), pos + Vec3.new( size.x,  size.y,  size.z), pos + Vec3.new(-size.x,  size.y,  size.z), Vec2.new(0.0,  0.0), Vec2.new(uv.x, 0.0),  Vec2.new(uv.x, uv.y), Vec2.new(0.0,  uv.y))
    /*-x*/mesh.addQuad(pos + Vec3.new(-size.x, -size.y,  size.z), pos + Vec3.new(-size.x,  size.y,  size.z), pos + Vec3.new(-size.x,  size.y, -size.z), pos + Vec3.new(-size.x, -size.y, -size.z), Vec2.new(uv.z, 0.0), Vec2.new(uv.z, uv.y), Vec2.new(0.0,  uv.y), Vec2.new(0.0,  0.0))
    /*+x*/mesh.addQuad(pos + Vec3.new( size.x, -size.y, -size.z), pos + Vec3.new( size.x,  size.y, -size.z), pos + Vec3.new( size.x,  size.y,  size.z), pos + Vec3.new( size.x, -size.y,  size.z), Vec2.new(0.0,  0.0), Vec2.new(0.0,  uv.y), Vec2.new(uv.z, uv.y), Vec2.new(uv.z, 0.0))    
    /*+y*/mesh.addQuad(pos + Vec3.new(-size.x,  size.y, -size.z), pos + Vec3.new(-size.x,  size.y,  size.z), pos + Vec3.new( size.x,  size.y,  size.z), pos + Vec3.new( size.x,  size.y, -size.z), Vec2.new(0.0,  0.0), Vec2.new(0.0,  uv.z), Vec2.new(uv.x, uv.z), Vec2.new(uv.x, 0.0))
    /*-y*/mesh.addQuad(pos + Vec3.new( size.x, -size.y, -size.z), pos + Vec3.new( size.x, -size.y,  size.z), pos + Vec3.new(-size.x, -size.y,  size.z), pos + Vec3.new(-size.x, -size.y, -size.z), Vec2.new(uv.x, 0.0), Vec2.new(uv.x, uv.z), Vec2.new(0.0,  uv.z), Vec2.new(0.0,  0.0))
  }

  //             __
  //          __|  |__
  //         |        |
  //       __|        |__
  //      |              |
  //      |              |
  //      |              |
  //    __|              |__
  //   |                    |
  //   |                    |
  //   |                    |
  //   |                    |
  //   |                    |
  // __|                    |__
  //|                          |
  //|                          |
  //|__________________________|
  spawnBuilding01(min, max, scale) {
    var size = (max - min)
    var center = (max + min) / 2
    
    var steps = [ _maxHeight * 0.2, _maxHeight * 0.7, _maxHeight * 0.825, _maxHeight * 0.95, _maxHeight * 1.0 ]
    for (i in 0...steps.count) {
      var prev = i == 0 ? 0 : steps[i - 1]
      var curr = steps[i]
      var size = size * ((steps.count - i) / steps.count) / 2

      var min = Vec3.new(center.x - size.x, prev, center.z - size.z)
      var max = Vec3.new(center.x + size.x, curr, center.z + size.z)
      spawnCube(min, max, scale, _meshBuildings)
    }
  }

  // ______     ______
  //|      |   |      |
  //|      |   |      |
  //|      |   |      |
  //|      |___|      |
  //|                 |
  //|                 |
  //|       ___       |
  //|      |   |      |
  //|      |   |      |
  //|      |   |      |
  //|      |   |      |
  //|      |   |      |
  //|      |   |      |
  //|      |   |      |
  //|      |   |      |
  //|______|   |______|
  spawnBuilding02(min, max, scale) {
    max.y = _maxHeight
    var size = (max - min)
    var center = (max + min) / 2

    var leftMin    = Vec3.new(min.x, min.y, min.z)
    var leftMax    = Vec3.new(max.x, max.y, min.z + size.z * 0.4)
    var rightMin   = Vec3.new(min.x, min.y, max.z - size.z * 0.4)
    var rightMax   = Vec3.new(max.x, max.y, max.z)
    var centerMin  = Vec3.new(min.x, min.y, min.z + size.z * 0.4)
    var centerMax  = Vec3.new(max.x, min.y, max.z - size.z * 0.4)
    var walkwayMin = Vec3.new(min.x + size.x * 0.2, (min.y + max.y) / 2, min.z + size.z * 0.4)
    var walkwayMax = Vec3.new(max.x - size.x * 0.2, (min.y + max.y) / 2 + 20, max.z - size.z * 0.4)
    spawnCube(leftMin, leftMax, scale, _meshBuildings)
    constructSidewalk(centerMin, centerMax)
    spawnCube(rightMin, rightMax, scale, _meshBuildings)
    spawnCube(walkwayMin, walkwayMax, scale, _meshBuildings)
  }

  // _________________
  //|    |       |    |
  //|    |       |    |
  //|    |       |    |
  //|    |       |    |
  //|    |       |    |
  //|    |       |    |
  //|    |       |    |
  //|    |       |    |
  //|    |       |    |
  //|    |       |    |
  //|    |       |    |
  //|    |       |    |
  //|    |       |    |
  //|    |   _   |    |
  //|    |  | |  |    |
  //|____|_|___|_|____|
  spawnBuilding03(min, max, scale) {
    max.y = _maxHeight
    var size = (max - min)
    var center = (max + min) / 2

    var leftMin   = Vec3.new(min.x, min.y, min.z)
    var leftMax   = Vec3.new(max.x, max.y, min.z + size.z * 0.2)
    var rightMin  = Vec3.new(min.x, min.y, max.z - size.z * 0.2)
    var rightMax  = Vec3.new(max.x, max.y, max.z)
    var centerMin = Vec3.new(min.x, min.y, min.z + size.z * 0.2)
    var centerMax = Vec3.new(max.x - size.x * 0.4, min.y, max.z - size.z * 0.2)
    var backMin   = Vec3.new(max.x - size.x * 0.4, min.y, min.z + size.z * 0.2)
    var backMax   = Vec3.new(max.x, max.y, max.z - size.z * 0.2)
    spawnCube(leftMin, leftMax, scale, _meshBuildings)
    constructSidewalk(centerMin, centerMax)
    spawnCube(rightMin, rightMax, scale, _meshBuildings)
    spawnCube(backMin, backMax, scale, _meshBuildings)

    var fcenter = center - Vec3.new(size.x * 0.25, 0.0, 0.0)
    var fmin = fcenter - size * 0.1
    var fmax = fcenter + size * 0.1

    spawnCube(Vec3.new(fmin.x, min.y, fmin.z), Vec3.new(fmin.x + 1, min.y + 2, fmax.z), scale, _meshBuildings)
    spawnCube(Vec3.new(fmax.x - 1, min.y, fmin.z), Vec3.new(fmax.x, min.y + 2, fmax.z), scale, _meshBuildings)
    spawnCube(Vec3.new(fmin.x + 1, min.y, fmin.z), Vec3.new(fmax.x - 1, min.y + 2, fmin.z + 1), scale, _meshBuildings)
    spawnCube(Vec3.new(fmin.x + 1, min.y, fmax.z - 1), Vec3.new(fmax.x - 1, min.y + 2, fmax.z), scale, _meshBuildings)
    spawnCube(Vec3.new(fcenter.x - 0.5, min.y, fcenter.z - 0.5), Vec3.new(fcenter.x + 0.5, min.y + 2, fcenter.z + 0.5), scale, _meshBuildings)
    spawnCube(Vec3.new(fcenter.x - 1.0, min.y + 2, fcenter.z - 1.0), Vec3.new(fcenter.x + 1.0, min.y + 5, fcenter.z + 1.0), scale, _meshBuildings)
  }

  spawnBuildingBlock(fullMin, fullMax, num, scale) {
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

  spawnBlock(fullMin, fullMax, num, scale) {
    var rand = Math.random()
    if (rand < 0.25) return spawnBuilding01(fullMin, fullMax, scale)
    if (rand < 0.50) return spawnBuilding02(fullMin, fullMax, scale)
    if (rand < 0.75) return spawnBuilding03(fullMin, fullMax, scale)
    if (rand < 1.00) return spawnBuildingBlock(fullMin, fullMax, num, scale)
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

  constructLights() {
    var distance = Vec2.new(
      _blockSize.x - (_streetSize.x - _sidewalkSize.x),
      _blockSize.y - (_streetSize.y - _sidewalkSize.y)
    )
    var numLanterns = Vec2.new(
      2,//(distance.x / (_streetSize.x * 2.0)).round,
      2//(distance.y / (_streetSize.y * 2.0)).round
    )
    var lanternDistance = Vec2.new(
      distance.x / numLanterns.x,
      distance.y / numLanterns.y
    )

    for (z in (-_numBlocks.y / 2)...(_numBlocks.y / 2)) {
      var zOffset = z * _blockSize.y - (_blockSize.y * 0.5)
      for (x in (-_numBlocks.x / 2)...(_numBlocks.x / 2)) {
        var xOffset = x * _blockSize.x - (_blockSize.x * 0.5)

        for (vx in 0..numLanterns.x) {
          var vxOffset = xOffset + vx * lanternDistance.x + (_streetSize.x - _sidewalkSize.x) / 2.0
          if (vx != 0) constructLantern(Vec3.new(vxOffset, 0.0, zOffset - (_streetSize.y - _sidewalkSize.y) / 2.0))
          constructLantern(Vec3.new(vxOffset, 0.0, zOffset + (_streetSize.y - _sidewalkSize.y) / 2.0))
        }
        
        for (vz in 0..numLanterns.y) {
          var vzOffset = zOffset + vz * lanternDistance.y + (_streetSize.y - _sidewalkSize.y) / 2.0
          if (vz != 0) constructLantern(Vec3.new(xOffset - (_streetSize.x - _sidewalkSize.x) / 2.0, 0.0, vzOffset))
          if (vz != 0) constructLantern(Vec3.new(xOffset + (_streetSize.x - _sidewalkSize.x) / 2.0, 0.0, vzOffset))
        }
      }
    }
  }

  constructTriNavMesh() {
    _triNavMesh = TriNavMesh.new()
  }
  
  addTriNavMeshQuad(min, max) {
    _triNavMesh.addQuad(min, max)
    return

    var scale = 4
    min.x = (min.x / scale).round * scale
    min.y = (min.y / scale).round * scale
    min.z = (min.z / scale).round * scale
    max.x = (max.x / scale).round * scale
    max.y = (max.y / scale).round * scale
    max.z = (max.z / scale).round * scale
    var splitsX = Math.max(((max.x - min.x) / scale).round, 1)
    var splitsZ = Math.max(((max.z - min.z) / scale).round, 1)

    for (z in 0...splitsZ) {
      for (x in 0...splitsX) {
        var a = Vec3.new(min.x + (x + 0) * scale, min.y, min.z + (z + 1) * scale)
        var b = Vec3.new(min.x + (x + 1) * scale, min.y, min.z + (z + 1) * scale)
        var c = Vec3.new(min.x + (x + 1) * scale, min.y, min.z + (z + 0) * scale)
        var d = Vec3.new(min.x + (x + 0) * scale, min.y, min.z + (z + 0) * scale)
        _triNavMesh.addTri(a, b, c)
        _triNavMesh.addTri(a, c, d)
      }  
    }
  }

  drawTris() {
    var tris = _triNavMesh.getTriangles()
    for (i in 0...(tris.count / 2)) {
      var a = tris[i * 2 + 0]
      var b = tris[i * 2 + 1]
      Debug.drawLine(a, b, Vec4.new(1.0))
    }
  }
}