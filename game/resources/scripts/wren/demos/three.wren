import "Core" for Vec2, Vec3, Vec4
import "Core" for Texture, Shader, Mesh
import "Core" for GameObject, Transform, Camera, CameraProjection, MeshRender, Lod, RigidBody, WaveSource, Collider, MonoBehaviour
import "Core" for Input, Keys, Buttons, Axes
import "Core" for Math, Graphics, GUI, Time, File, Assert, PostProcess, Console, Physics, Debug, Sort, PhysicsConstraints
import "Core" for NavMesh, Noise, NoiseInterpolation

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
import "resources/scripts/wren/meshes" for Meshes, MeshCreator

class Square {
    construct new(topLeft, topRight, bottomRight, bottomLeft) {
        _topLeft     = topLeft
        _topRight    = topRight
        _bottomRight = bottomRight
        _bottomLeft  = bottomLeft

        _centreTop    = topLeft.right
        _centreRight  = bottomRight.above
        _centreBottom = bottomLeft.right
        _centreLeft   = bottomLeft.above

        _configuration = 0

        if (topLeft.active)     _configuration = _configuration + 8
        if (topRight.active)    _configuration = _configuration + 4
        if (bottomRight.active) _configuration = _configuration + 2
        if (bottomLeft.active)  _configuration = _configuration + 1
    }

    topLeft { _topLeft.pos }
    topRight { _topRight.pos }
    bottomRight { _bottomRight.pos }
    bottomLeft { _bottomLeft.pos }
    centreTop { _centreTop.pos }
    centreRight { _centreRight.pos }
    centreBottom { _centreBottom.pos }
    centreLeft { _centreLeft.pos }
    configuration { _configuration }
}

class Node {
    construct new(pos) {
        _pos = pos
    }

    pos { _pos }
}

class ControlNode is Node {
    construct new(pos, active, squareSize) {
        super(pos)
        _active = active
        _above = Node.new(pos + Vec3.new(1.0, 0.0, 0.0) * squareSize / 2.0)
        _right = Node.new(pos + Vec3.new(0.0, 0.0, 1.0) * squareSize / 2.0)
    }

    right { _right }
    above { _above }
    active { _active }
}

class MyMap {
    construct new() {
        constructMap(50, 50, 1, 0.5)
    }

    getValue(x, y) {
        return _map[y][x]
    }

    fillMap() {
        _map = List.filled(_height, null)
        for (i in 0..._height) _map[i] = List.filled(_width, 0)

        for (y in 0..._height) {
            for (x in 0..._width) {
                if (x == 0 || x == _width - 1 || y == 0 || y == _height - 1) {
                    _map[y][x] = 1.0
                } else {
                    _map[y][x] = Math.random() < _fillPercentage ? 1.0 : 0.0
                }
            }
        }
    }

    getNeighbourCount(x, y) {
        var neighbourCount = 0

        for (dx in -1..1) {
            for (dy in -1..1) {
                var ddx = x + dx
                var ddy = y + dy
                if (!(dx == 0 && dy == 0) && ddx >= 0 && ddx < _width && ddy >= 0 && ddy < _height && _map[ddy][ddx] > 0.0) {
                    neighbourCount = neighbourCount + 1
                }
            }
        }

        return neighbourCount
    }

    smoothMap() {
        for (y in 0..._height) {
            for (x in 0..._width) {
                var numNeighbours = getNeighbourCount(x, y)
                if (numNeighbours > 4) _map[y][x] = 1.0
                if (numNeighbours < 4) _map[y][x] = 0.0
            }
        }
    }
    
    triangulateSquare(square) {
		if (square.configuration == 0)  return
        // 1 points:
		if (square.configuration == 1)  return meshFromPoints(square.centreBottom, square.bottomLeft, square.centreLeft)
		if (square.configuration == 2)  return meshFromPoints(square.centreRight, square.bottomRight, square.centreBottom)
        if (square.configuration == 4)  return meshFromPoints(square.centreTop, square.topRight, square.centreRight)
		if (square.configuration == 8)  return meshFromPoints(square.topLeft, square.centreTop, square.centreLeft)

		// 2 points:
		if (square.configuration == 3)  return meshFromPoints(square.centreRight, square.bottomRight, square.bottomLeft, square.centreLeft)
		if (square.configuration == 6)  return meshFromPoints(square.centreTop, square.topRight, square.bottomRight, square.centreBottom)
		if (square.configuration == 9)  return meshFromPoints(square.topLeft, square.centreTop, square.centreBottom, square.bottomLeft)
		if (square.configuration == 12) return meshFromPoints(square.topLeft, square.topRight, square.centreRight, square.centreLeft)
		if (square.configuration == 5)  return meshFromPoints(square.centreTop, square.topRight, square.centreRight, square.centreBottom, square.bottomLeft, square.centreLeft)
		if (square.configuration == 10) return meshFromPoints(square.topLeft, square.centreTop, square.centreRight, square.bottomRight, square.centreBottom, square.centreLeft)

		// 3 point:
        if (square.configuration == 7)  return meshFromPoints(square.centreTop, square.topRight, square.bottomRight, square.bottomLeft, square.centreLeft)
        if (square.configuration == 11) return meshFromPoints(square.topLeft, square.centreTop, square.centreRight, square.bottomRight, square.bottomLeft)
        if (square.configuration == 13) return meshFromPoints(square.topLeft, square.topRight, square.centreRight, square.centreBottom, square.bottomLeft)
        if (square.configuration == 14) return meshFromPoints(square.topLeft, square.topRight, square.bottomRight, square.centreBottom, square.centreLeft)

		// 4 point:
		if (square.configuration == 15) return meshFromPoints(square.topLeft, square.topRight, square.bottomRight, square.bottomLeft)
	}

	meshFromPoints(a, b, c) {
        _mesh.addTriDS(a, b, c, Vec2.new(0.0), Vec2.new(0.0), Vec2.new(0.0))
	}
    meshFromPoints(a, b, c, d) {
        _mesh.addTriDS(a, b, c, Vec2.new(0.0), Vec2.new(0.0), Vec2.new(0.0))
        _mesh.addTriDS(a, c, d, Vec2.new(0.0), Vec2.new(0.0), Vec2.new(0.0))
	}
    meshFromPoints(a, b, c, d, e) {
        _mesh.addTriDS(a, b, c, Vec2.new(0.0), Vec2.new(0.0), Vec2.new(0.0))
        _mesh.addTriDS(a, c, d, Vec2.new(0.0), Vec2.new(0.0), Vec2.new(0.0))
        _mesh.addTriDS(a, d, e, Vec2.new(0.0), Vec2.new(0.0), Vec2.new(0.0))
	}
    meshFromPoints(a, b, c, d, e, f) {
        _mesh.addTriDS(a, b, c, Vec2.new(0.0), Vec2.new(0.0), Vec2.new(0.0))
        _mesh.addTriDS(a, c, d, Vec2.new(0.0), Vec2.new(0.0), Vec2.new(0.0))
        _mesh.addTriDS(a, d, e, Vec2.new(0.0), Vec2.new(0.0), Vec2.new(0.0))
        _mesh.addTriDS(a, e, f, Vec2.new(0.0), Vec2.new(0.0), Vec2.new(0.0))
	}

    constructMap(width, height, scale, fillPercentage) {
        _width  = width
        _height = height
        _scale  = scale
        _fillPercentage = fillPercentage

        fillMap()

        for (i in 0...5) {
            smoothMap()
        }
        
        var mapWidth = _width * _scale
        var mapHeight = _height * _scale

        var controlNodes = List.filled(_height, null)
        for (i in 0..._height) controlNodes[i] = List.filled(_width, null)

        for (y in 0..._height) {
            for (x in 0..._width) {
                var pos = Vec3.new(-mapWidth / 2 + x * _scale + _scale / 2, 0, -mapHeight / 2 + y * _scale + _scale / 2)
                controlNodes[y][x] = ControlNode.new(pos, _map[y][x] > 0.0, _scale)
            }
        }

        var squares = List.filled(_height - 1, null)
        for (i in 0...(_height - 1)) squares[i] = List.filled(_width - 1, null)
        for (x in 0...(_width - 1)) {
            for (y in 0...(_height - 1)) {
                squares[y][x] = Square.new(controlNodes[y + 1][x], controlNodes[y + 1][x + 1], controlNodes[x + 1][y], controlNodes[x][y])
            }
        }

        _mesh = MeshCreator.new()

		for (x in 0...squares[0].count) {
			for (y in 0...squares.count) {
				triangulateSquare(squares[y][x])
			}
		}

        // var scaleVec = Vec3.new(_scale, 10.0, _scale)
        // var offset = Vec3.new(width * 0.5, 0.0, _height * 0.5) * scaleVec
        // for (y in 0...(_height - 1)) {
        //     for (x in 0...(_width - 1)) {
        //         var a = Vec3.new(x,     getValue(y,     x),     y)     * scaleVec - offset
        //         var b = Vec3.new(x + 1, getValue(y,     x + 1), y)     * scaleVec - offset
        //         var c = Vec3.new(x + 1, getValue(y + 1, x + 1), y + 1) * scaleVec - offset
        //         var d = Vec3.new(x,     getValue(y + 1, x),     y + 1) * scaleVec - offset
        //         var uvA = Vec2.new(0.0,            0.0)
        //         var uvB = Vec2.new((b - a).length, 0.0)
        //         var uvC = Vec2.new((d - c).length, (b - c).length)
        //         var uvD = Vec2.new(0.0,            (a - d).length)
                
        //         _mesh.addQuad(d, c, b, a, uvD, uvC, uvB, uvA)
        //     }
        // }

        makeGameObject(_mesh.make)
        _mesh = null
    }

    makeGameObject(mesh) {
        _go = GameObject.new()
        _go.name = "MyMap"
        
        var meshRender = _go.addComponent(MeshRender)
        meshRender.mesh    = mesh
        meshRender.subMesh = 0
        meshRender.albedo  = Texture.load("resources/textures/asphalt/Asphalt10_alb.jpg")
        meshRender.normal  = Texture.load("resources/textures/asphalt/Asphalt10_nrm.jpg")
        meshRender.DMRA    = Texture.load("resources/textures/asphalt/Asphalt10_dmra.png")
        meshRender.makeStatic()

        var collider = _go.addComponent(Collider)
        collider.makeMeshCollider(meshRender.mesh, 0)
        collider.layers = PhysicsLayers.General
    }
}

class Demo {
  construct new() {
  }

  delayedInitialize() {
    _myMap = MyMap.new()

    GUI.executeJavaScript("LoadWebPage('file:///resources/web-pages/menu.html')")
  }

  initialize() {
    _post_processer = PostProcessor.new()
    _lighting = Lighting.new()

    _camera = GameObject.new()
    _camera.addComponent(ThirdPersonCamera)

    GUI.loadURL("file:///resources/web-pages/ui.html")
    GUI.executeJavaScript("LoadWebPage('file:///resources/web-pages/loading_screen.html')")

    _frameIndex = -1
  }
  deinitialize() {

  }
  update() {
    _frameIndex = _frameIndex + 1
    if (_frameIndex == 0) return
    if (_frameIndex == 1) return delayedInitialize()

    GUI.executeJavaScript("if (gameState == Game) { updateDebug(%(Time.deltaTime)) }")
    
    updateSun()
  }

  updateSun() {
    // Update the RSM light.
    var sin_y_rot = -Math.sin(Math.deg2Rad * _camera.getComponent(FreeLookCamera).rotation.y)
    var cos_y_rot = -Math.cos(Math.deg2Rad * _camera.getComponent(FreeLookCamera).rotation.y)
    var offset = Vec3.new(sin_y_rot, 0.0, cos_y_rot).normalized * 12.5
    var transform = _lighting.rsm.getComponent(Transform)
    transform.worldPosition = _camera.transform.worldPosition + offset


    //_camera.getComponent(ThirdPersonCamera).cameraTransform.gameObject.getComponent(Camera).height = 32.0
    //_camera.getComponent(ThirdPersonCamera).cameraTransform.gameObject.getComponent(Camera).width = _camera.getComponent(ThirdPersonCamera).cameraTransform.gameObject.getComponent(Camera).height * Graphics.aspectRatio
    //_camera.getComponent(ThirdPersonCamera).cameraTransform.gameObject.getComponent(Camera).projection = CameraProjection.Ortho


    var dir = Vec3.new(5.0, 25.0, 0.0)
    _camera.getComponent(ThirdPersonCamera).transformInBetween.localPosition = Vec3.new(0.0)
    _camera.getComponent(ThirdPersonCamera).transformInBetween.worldEuler = Vec3.new(0.0)
    _camera.getComponent(ThirdPersonCamera).cameraTransform.worldRotation = Math.lookRotation(dir, Vec3.new(0.0, 1.0, 0.0))
    _camera.getComponent(ThirdPersonCamera).cameraTransform.localPosition = dir
  }

  fixedUpdate() {
  }
}
