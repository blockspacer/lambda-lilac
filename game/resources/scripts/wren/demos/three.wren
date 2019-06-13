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

class RoomSorter {
    static greater(a, b) { (a.roomSize > b.roomSize) }
}

class Room {
    tiles { _tiles }
    edgeTiles { _edgeTiles }
    connectedRooms { _connectedRooms }
    roomSize { _roomSize }
    isAccessibleFromMainRoom { _isAccessibleFromMainRoom }

    construct new(roomTiles, map) {
        _tiles = roomTiles
        _roomSize = tiles.count
        _connectedRooms = []
        _isMainRoom = false
        _isAccessibleFromMainRoom = false

        _edgeTiles = []
        for (tile in tiles) {
            for (x in (tile.x - 1)..(tile.x + 1)) {
                for (y in (tile.y - 1)..(tile.y + 1)) {
                    if (x == tile.x || y == tile.y) {
                        if (y >= 0 && y < map.count && x >= 0 && x < map[0].count && map[y][x] == 1) {
                            _edgeTiles.add(tile)
                        }
                    }
                }
            }
        }
    }
    
    setMainRoom() {
        _isMainRoom = true
        _isAccessibleFromMainRoom = true
    }

    setAccessibleFromMainRoom() {
        if (!_isAccessibleFromMainRoom) {
            _isAccessibleFromMainRoom = true
            for (connectedRoom in connectedRooms) {
                connectedRoom.setAccessibleFromMainRoom()
            }
        }
    }

    connectRoom(room) {
        if (_isAccessibleFromMainRoom) room.setAccessibleFromMainRoom()
        _connectedRooms.add(room)
    }

    isConnected(otherRoom) {
        return _connectedRooms.contains(otherRoom)
    }
}

class Line {
    construct new(from, to, color) {
        _from  = from
        _to    = to
        _color =  color
    }

    draw { Debug.drawLine(_from, _to, _color) }
}

class MyMap {
    construct new() {
        constructMap(75, 75, 2.5, 0.5)
    }

    getValue(x, y) {
        if (x < 0 || y < 0 || x >= _width || y >= _height) {
            return 1.0
        } else {
            return _map[y][x]
        }
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
                if (y > 1 && y < _height - 1 && x > 1 && x < _width - 1) {
                    var numNeighbours = getNeighbourCount(x, y)
                    if (numNeighbours > 4) _map[y][x] = 1.0
                    if (numNeighbours < 4) _map[y][x] = 0.0
                }
            }
        }
    }

    addTriAuto(a, b, c) {
        _mesh.addTriAuto(a, b, c)
        var up = (c - a).normalized
        var r1 = up.cross(Vec3.new(0.0, 1.0, 0.0))
        var r2 = up.cross(Vec3.new(0.0, 0.0, 1.0))
        var right = (r1.lengthSqr > r2.lengthSqr) ? r1 : r2

        _lines.add(Line.new(a, a + up, Vec4.new(1.0, 0.0, 0.0, 1.0)))
        _lines.add(Line.new(a, a + right, Vec4.new(0.0, 0.0, 1.0, 1.0)))
    }

    meshFromPoints(a, b, c) {
        addTriAuto(a, b, c)
	}

    meshFromPoints(a, b, c, d) {
        addTriAuto(a, b, c)
        addTriAuto(a, c, d)
    }
    
    meshFromPoints(a, b, c, d, e) {
        addTriAuto(a, b, c)
        addTriAuto(a, c, d)
        addTriAuto(a, d, e)
    }
    
    meshFromPoints(a, b, c, d, e, f) {
        addTriAuto(a, b, c)
        addTriAuto(a, c, d)
        addTriAuto(a, d, e)
        addTriAuto(a, e, f)
    }
    
    addMeshCell(x, y, height, value) {
        var bTopLeft     = getValue(x,     y)     == value
        var bTopRight    = getValue(x + 1, y)     == value
        var bBottomRight = getValue(x + 1, y - 1) == value
        var bBottomLeft  = getValue(x,     y - 1) == value
        
        var e = Vec3.new(_scale, 0.0, _scale)
        var c = Vec3.new(x - _width * 0.5, height, y - _height * 0.5) * _scale
        var topLeft      = c + Vec3.new(       0.0, 0.0,        0.0)
        var centreTop    = c + Vec3.new( e.x * 0.5, 0.0,        0.0)
        var topRight     = c + Vec3.new(       e.x, 0.0,        0.0)
        var centreLeft   = c + Vec3.new(       0.0, 0.0, -e.z * 0.5)
        var centreRight  = c + Vec3.new(       e.x, 0.0, -e.z * 0.5)
        var bottomLeft   = c + Vec3.new(       0.0, 0.0,       -e.z)
        var centreBottom = c + Vec3.new( e.x * 0.5, 0.0,       -e.z)
        var bottomRight  = c + Vec3.new(       e.x, 0.0,       -e.z)

        var config = 0
        if (bTopLeft)     config = config + 8
        if (bTopRight)    config = config + 4
        if (bBottomRight) config = config + 2
		if (bBottomLeft)  config = config + 1

        if (height == 0) {
            addMeshCell(x, y, _scale * 2.0, value == 0 ? 1 : 0)
        }

        if (config == 0)  return

		// 1 points:
		if (config == 1) {
            meshFromPoints(centreLeft, centreBottom, bottomLeft)
            if (height != 0) {
               var a = Vec3.new(centreBottom.x, 0.0, centreBottom.z)
               var b = Vec3.new(centreLeft.x, 0.0, centreLeft.z)
               meshFromPoints(b, a, centreBottom, centreLeft)
            }
            return
        } 
		if (config == 2) {
            meshFromPoints(bottomRight, centreBottom, centreRight)
            if (height != 0) {
               var a = Vec3.new(centreRight.x, 0.0, centreRight.z)
               var b = Vec3.new(centreBottom.x, 0.0, centreBottom.z)
               meshFromPoints(b, a, centreRight, centreBottom)
            }
            return
        } 
		if (config == 4) {
            meshFromPoints(topRight, centreRight, centreTop)
            if (height != 0) {
               var a = Vec3.new(centreTop.x, 0.0, centreTop.z)
               var b = Vec3.new(centreRight.x, 0.0, centreRight.z)
               meshFromPoints(b, a, centreTop, centreRight)
            }
            return
        } 
		if (config == 8) {
            meshFromPoints(topLeft, centreTop, centreLeft)
            if (height != 0) {
               var a = Vec3.new(centreLeft.x, 0.0, centreLeft.z)
               var b = Vec3.new(centreTop.x, 0.0, centreTop.z)
               meshFromPoints(b, a, centreLeft, centreTop)
            }
            return
        } 

		// 2 points:
		if (config == 3) {
            meshFromPoints(centreRight, bottomRight, bottomLeft, centreLeft)
            if (height != 0) {
               var a = Vec3.new(centreRight.x, 0.0, centreRight.z)
               var b = Vec3.new(centreLeft.x, 0.0, centreLeft.z)
               meshFromPoints(b, a, centreRight, centreLeft)
            }
            return
        } 
		if (config == 6) {
            meshFromPoints(centreTop, topRight, bottomRight, centreBottom)
            if (height != 0) {
               var a = Vec3.new(centreTop.x, 0.0, centreTop.z)
               var b = Vec3.new(centreBottom.x, 0.0, centreBottom.z)
               meshFromPoints(b, a, centreTop, centreBottom)
            }
            return
        } 
		if (config == 9) {
            meshFromPoints(topLeft, centreTop, centreBottom, bottomLeft)
            if (height != 0) {
               var a = Vec3.new(centreBottom.x, 0.0, centreBottom.z)
               var b = Vec3.new(centreTop.x, 0.0, centreTop.z)
               meshFromPoints(b, a, centreBottom, centreTop)
            }
            return
        } 
		if (config == 12) {
            meshFromPoints(topLeft, topRight, centreRight, centreLeft)
            if (height != 0) {
               var a = Vec3.new(centreLeft.x, 0.0, centreLeft.z)
               var b = Vec3.new(centreRight.x, 0.0, centreRight.z)
               meshFromPoints(b, a, centreLeft, centreRight)
            }
            return
        } 
		if (config == 5) {
            meshFromPoints(centreTop, topRight, centreRight, centreBottom, bottomLeft, centreLeft)
            if (height != 0) {
            }
            return
        } 
		if (config == 10) {
            meshFromPoints(topLeft, centreTop, centreRight, bottomRight, centreBottom, centreLeft)
            if (height != 0) {
            }
            return
        } 

		// 3 point:
		if (config == 7)  {
            meshFromPoints(centreTop, topRight, bottomRight, bottomLeft, centreLeft)
            if (height != 0) {
                var a = Vec3.new(centreTop.x, 0.0, centreTop.z)
                var b = Vec3.new(centreLeft.x, 0.0, centreLeft.z)
                meshFromPoints(b, a, centreTop, centreLeft)
            }
            return
        } 
		if (config == 11) {
            meshFromPoints(topLeft, centreTop, centreRight, bottomRight, bottomLeft)
            if (height != 0) {
               var a = Vec3.new(centreRight.x, 0.0, centreRight.z)
               var b = Vec3.new(centreTop.x, 0.0, centreTop.z)
               meshFromPoints(b, a, centreRight, centreTop)
            }
            return
        }
		if (config == 13) {
            meshFromPoints(topLeft, topRight, centreRight, centreBottom, bottomLeft)
            if (height != 0) {
                var a = Vec3.new(centreBottom.x, 0.0, centreBottom.z)
                var b = Vec3.new(centreRight.x, 0.0, centreRight.z)
                meshFromPoints(b, a, centreBottom, centreRight)
            }
            return
        }
		if (config == 14) {
            meshFromPoints(topLeft, topRight, bottomRight, centreBottom, centreLeft)
            if (height != 0) {
                var a = Vec3.new(centreLeft.x, 0.0, centreLeft.z)
                var b = Vec3.new(centreBottom.x, 0.0, centreBottom.z)
                meshFromPoints(b, a, centreLeft, centreBottom)
            }
            return
        }

		// 4 point:
		if (config == 15) {
            return meshFromPoints(topLeft, topRight, bottomRight, bottomLeft)
        }

        Assert.throw("Config: %(config) is not supported! Using seed: %(_seed)")
    }

	processMap() {
		var wallRegions = getRegions(1)
		var wallThresholdSize = 50

		for (wallRegion in wallRegions) {
			if (wallRegion.count < wallThresholdSize) {
				for (tile in wallRegion) {
					_map[tile.y][tile.x] = 0
				}
			}
		}

		var roomRegions = getRegions(0)
		var roomThresholdSize = 50
		var survivingRooms = []
		
		for (roomRegion in roomRegions) {
			if (roomRegion.count < roomThresholdSize) {
				for (tile in roomRegion) {
					_map[tile.y][tile.x] = 1
				}
			} else {
				survivingRooms.add(Room.new(roomRegion, _map))
			}
		}

        survivingRooms = Sort.sort(survivingRooms, RoomSorter)
        survivingRooms[0].setMainRoom()
        
		connectClosestRooms(survivingRooms, false)
	}

    connectClosestRooms(allRooms, forceAccessibilityFromMainRoom) {
		var roomListA = []
		var roomListB = []
		
		if (forceAccessibilityFromMainRoom) {
			for (room in allRooms) {
				if (room.isAccessibleFromMainRoom) {
					roomListB.add(room)
				} else {
					roomListA.add(room)
				}
			}
		} else {
			roomListA = allRooms
			roomListB = allRooms
		}
        
		var bestDistance = 0
		var bestTileA
		var bestTileB
		var bestRoomA
		var bestRoomB
		var possibleConnectionFound = false

		for (roomA in allRooms) {
            var canContinue = true
			if (!forceAccessibilityFromMainRoom) {
				possibleConnectionFound = false
				if (roomA.connectedRooms.count > 0) {
					canContinue = false
				}
			}

            if (canContinue) {
                for (roomB in allRooms) {
                    if (roomA == roomB || roomA.isConnected(roomB)) {
                        canContinue = false
                    }

                    if (canContinue) {
                        for (tileIndexA in 0...roomA.edgeTiles.count) {
                            for (tileIndexB in 0...roomB.edgeTiles.count) {
                                var tileA = roomA.edgeTiles[tileIndexA]
                                var tileB = roomB.edgeTiles[tileIndexB]
                                var distanceBetweenRooms = (Math.pow(tileA.x - tileB.x, 2) + Math.pow(tileA.y - tileB.y, 2))

                                if (distanceBetweenRooms < bestDistance || !possibleConnectionFound) {
                                    bestDistance = distanceBetweenRooms
                                    possibleConnectionFound = true
                                    bestTileA = tileA
                                    bestTileB = tileB
                                    bestRoomA = roomA
                                    bestRoomB = roomB
                                }
                            }
                        }
                    }
                }

                if (possibleConnectionFound && !forceAccessibilityFromMainRoom) {
                    createPassage(bestRoomA, bestRoomB, bestTileA, bestTileB)
                }
            }
		}
        
		if (possibleConnectionFound && forceAccessibilityFromMainRoom) {
			createPassage(bestRoomA, bestRoomB, bestTileA, bestTileB)
			connectClosestRooms(allRooms, true)
		}
		
		if (!forceAccessibilityFromMainRoom) {
			connectClosestRooms(allRooms, true)
		}
	}

	createPassage(roomA, roomB, tileA, tileB) {
        roomA.connectRoom(roomB)
        roomB.connectRoom(roomA)

        var line = []

        var len = (tileB - tileA).length
        var dir = (tileB - tileA).normalized

        for (i in 0...len.round) {
			drawCircle(tileA + dir * i, 2)
        }
	}

	getRegions(tileType) {
		var regions = []
		var mapFlags = List.filled(_height, null)
        for (i in 0..._height) mapFlags[i] = List.filled(_width, 0)

		for (x in 0..._width) {
			for (y in 0..._height) {
				if (mapFlags[y][x] == 0 && _map[y][x] == tileType) {
					var newRegion = getRegionTiles(x,y)
					regions.add(newRegion)

					for (tile in newRegion) {
						mapFlags[tile.y][tile.x] = 1
					}
				}
			}
		}

		return regions
	}
    
	getRegionTiles(startX, startY) {
		var tiles = []
		var mapFlags = List.filled(_height, null)
        for (i in 0..._height) mapFlags[i] = List.filled(_width, 0)
		var tileType = _map[startY][startX]

		var queue = []
		queue.add(Vec2.new(startX, startY))
		mapFlags[startY][startX] = 1

		while (queue.count > 0) {
			var tile = queue[0]
            queue.removeAt(0)
			tiles.add(tile)

			for (x in (tile.x - 1)..(tile.x + 1)) {
				for (y in (tile.y - 1)..(tile.y + 1)) {
					if (isInMapRange(x,y) && (y == tile.y || x == tile.x)) {
						if (mapFlags[y][x] == 0 && _map[y][x] == tileType) {
							mapFlags[y][x] = 1
							queue.add(Vec2.new(x,y))
						}
					}
				}
			}
		}

		return tiles
	}

	isInMapRange(x, y) {
		return x >= 0 && x < _width && y >= 0 && y < _height
	}

    drawCircle(c, r) {
		for (x in (-r)..r) {
			for (y in (-r)..r) {
				if (x * x + y * y <= r * r) {
					var drawX = c.x + x
					var drawY = c.y + y
					if (drawX.round > 0 && drawX.round < _width - 1 && drawY.round > 0 && drawY.round < _height - 1) {
						_map[drawY.round][drawX.round] = 0
					}
				}
			}
		}
	}

    draw {
        for (line in _lines) {
            line.draw
        }
    }

    constructMap(width, height, scale, fillPercentage) {
        //_seed = Math.hash("Hilze")
        _seed = Math.random(0.0, 1000000.0)
        Math.seed = _seed
        _width  = width
        _height = height
        _scale  = scale
        _fillPercentage = fillPercentage
        _lines = []

        fillMap()

		for (y in 0..._height) {
            _map[y][0]  = 1
            _map[y][-1] = 1
        }
		for (x in 0..._width) {
            _map[0][x]  = 1
            _map[-1][x] = 1
        }

        for (i in 0...2) {
            smoothMap()
        }
        
        processMap()
        smoothMap()

        _mesh = MeshCreator.new()

		for (y in 1..._height) {
    		for (x in 0...(_width - 1)) {
                addMeshCell(x, y, 0.0, 0)
			}
		}
        
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
    _myMap.draw
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


    //var dir = Vec3.new(-1.0, 25.0, 0.0) * 0.25
    //_camera.getComponent(ThirdPersonCamera).transformInBetween.localPosition = Vec3.new(0.0)
    //_camera.getComponent(ThirdPersonCamera).transformInBetween.worldEuler = Vec3.new(0.0)
    //_camera.getComponent(ThirdPersonCamera).cameraTransform.worldRotation = Math.lookRotation(dir, Vec3.new(0.0, 1.0, 0.0))
    //_camera.getComponent(ThirdPersonCamera).cameraTransform.localPosition = dir
  }

  fixedUpdate() {
  }
}
