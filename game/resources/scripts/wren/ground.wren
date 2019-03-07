import "Core/Vec2" for Vec2
import "Core/Vec3" for Vec3

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

import "Core/Time" for Time
import "Core/File" for File
import "Core/Assert" for Assert

import "Core/MonoBehaviour" for MonoBehaviour

import "Core/PostProcess" for PostProcess
import "Core/Console" for Console
import "Core/Noise" for Noise, NoiseInterpolation

class GroundEnabledType {
    static Enabled    { 0 }
    static ZeroHeight { 1 }
    static Disabled   { 2 }
}

class Ground {
    construct new() {
      _tiles = []
      _gameObjects = []
      _allow_repeat = [ true, false ]
      _meshes = []
      _needs_box_collider = []
      _cubes = []
      _offset = Vec3.new(20, 2, 20)
    }

    addTile(x, y, z, type) {
      while(_tiles.count <= z) {
        _tiles.add([])
      }
      while(_tiles[z].count <= y) {
        _tiles[z].add([])
      }
      while(_tiles[z][y].count <= x) {
        _tiles[z][y].add(null)
      }

      _tiles[z][y][x] = type
    }

    createObj(position, size, mesh, needs_box_collider) {
      var model = GameObject.new()
      model.name = "ground"
      model.transform.worldScale = size
      model.transform.worldPosition = position
      model.addComponent(MeshRender).mesh = mesh
      model.getComponent(MeshRender).subMesh = 0
      model.getComponent(MeshRender).albedo = Texture.load("resources/textures/mossy-ground1-albedo.png")
      model.getComponent(MeshRender).normal = Texture.load("resources/textures/mossy-ground1-normal.png")
      model.getComponent(MeshRender).metallicRoughness = Texture.load("resources/textures/mossy-ground1-metallic-roughness.png")
      var collider = model.addComponent(Collider)
      if (needs_box_collider) {
        collider.makeBoxCollider()
      } else {
        collider.makeSphereCollider()
      }
      _gameObjects.add(model)
    }

    cubeValid(from, to, type) {
      for (z in from.z..to.z) {
        for (y in from.y..to.y) {
          for (x in from.x..to.x) {
            if (_tiles[z][y][x] != type) {
              return false
            }
          }
        }
      }
      return true
    }

    floodFillTile(x, y, z, type) {
      if (type == null) {
        return
      }

      var max_z = z
      for (z_ in z..._tiles.count) {
        if (cubeValid(Vec3.new(x, y, z), Vec3.new(x, y, z_), type)) {
          max_z = z_
        } else {
          break
        }
      }
      
      var max_x = x
      for (x_ in x..._tiles[z][y].count) {
        if (cubeValid(Vec3.new(x, y, z), Vec3.new(x_, y, max_z), type)) {
          max_x = x_
        } else {
          break
        }
      }
      
      for (z_ in z..max_z) {
        for (x_ in x..max_x) {
          _tiles[z_][y][x_] = null
        }
      }

      var needs_box_collider = _needs_box_collider[type]
      var mesh = _meshes[type]

      if (_allow_repeat[type]) {
        var scale = Vec3.new(max_x - x + 1, 1, max_z - z + 1) * 2
        var position = Vec3.new(x, y, z) * 2 + scale / 2 - _offset
        createObj(position, scale, mesh, needs_box_collider)
      } else {
        for (z_ in z..max_z) {
          for (x_ in x..max_x) {
            var scale = Vec3.new(1, 1, 1) * 2
            var position = Vec3.new(x_, y, z_) * 2 + scale / 2 - _offset
            createObj(position, scale, mesh, needs_box_collider)
          }
        }
      }
    }

    rasterize() {
      for (z in 0..._tiles.count) {
        for (y in 0..._tiles[z].count) {
          for (x in 0..._tiles[z][y].count) {
            if (_tiles[z][y][x] != null) {
              floodFillTile(x, y, z, _tiles[z][y][x])
            }
          }
        }
      }
    }

    cube(position, scale) {
      var c = GameObject.new()
      c.name = "cube"
      c.transform.worldScale = scale
      c.transform.worldPosition = position

      c.addComponent(MeshRender).mesh = _meshes[0]
      c.getComponent(MeshRender).subMesh = 0
      c.getComponent(MeshRender).albedo = Texture.load("resources/textures/rustediron-streaks_basecolor.png")
      c.getComponent(MeshRender).normal = Texture.load("resources/textures/rustediron-streaks_normal.png")
      c.getComponent(MeshRender).metallicRoughness = Texture.load("resources/textures/rustediron-streaks_metallic_roughness.png")
      var collider  = c.addComponent(Collider)
      var rigidBody = c.addComponent(RigidBody)
      collider.makeBoxCollider()
      collider.mass     = 2.5
      collider.friction = 2.0

      _cubes.add(c)
    }

    initialize() {
      _meshes.add(Mesh.generate("cube"))
      _needs_box_collider.add(true)
      _meshes.add(Mesh.generate("sphere"))
      _needs_box_collider.add(false)

      for (z in 0..20) {
        for (x in 0..20) {
          // Layer 0
          addTile(x, 0, z, 0)
          
          // Layer 1
          if (!(x == 15)) {
            addTile(x, 1, z, 0)
          }

          // Layer 2
          if (z == 0 || z == 20 || x == 0 || x == 20) {
            if (!(x == 15)) {
              addTile(x, 2, z, 1)
            }
          }
        }
      }

      rasterize()

      cube(Vec3.new(0.0, 2.0, -5.0) * 2, Vec3.new(1.9))
      cube(Vec3.new(5.0, 2.0, -5.0) * 2, Vec3.new(1.9))
      cube(Vec3.new(5.0, 2.0, -0.0) * 2, Vec3.new(1.0, 0.1, 4.0))
    }

    update() {
      for(c in _cubes) {
        if (c.transform.worldPosition.y < -10) {
          c.transform.worldPosition = Vec3.new(0.0, 2.0, -0.0) * 2
          c.getComponent(RigidBody).velocity = Vec3.new(0.0)
        }
      }
    }
}