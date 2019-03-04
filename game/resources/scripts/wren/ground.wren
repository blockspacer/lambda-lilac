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
    }

    addTile(x, y, z) {
      while(_tiles.count <= z) {
        _tiles.add([])
      }
      while(_tiles[z].count <= y) {
        _tiles[z].add([])
      }
      while(_tiles[z][y].count <= x) {
        _tiles[z][y].add(null)
      }

      var model = GameObject.new()
      model.name = "ground"
      model.transform.worldScale = Vec3.new(2)
      model.transform.worldPosition = Vec3.new(x - 10, y, z - 10) * 2
      model.addComponent(MeshRender).mesh = y == 0 ? _mesh : _mesh2
      model.getComponent(MeshRender).subMesh = 0
      model.getComponent(MeshRender).albedo = Texture.load("resources/textures/mossy-ground1-albedo.png")
      model.getComponent(MeshRender).normal = Texture.load("resources/textures/mossy-ground1-normal.png")
      model.getComponent(MeshRender).metallicRoughness = Texture.load("resources/textures/mossy-ground1-metallic-roughness.png")
      var collider = model.addComponent(Collider)
      if (y == 0) {
        collider.makeBoxCollider()
      } else {
        collider.makeSphereCollider()
      }
      //collider.friction = 2.5
      _tiles[z][y][x] = model
    }

    initialize() {
      _mesh = Mesh.generate("cube")
      _mesh2 = Mesh.generate("sphere")

      for (z in 0..20) {
        for (x in 0..20) {
          addTile(x, 0, z)
          if (z == 0 || z == 20 || x == 0 || x == 20) {
            addTile(x, 1, z)
          }
        }
      }

      _cube = GameObject.new()
      _cube.name = "cube"
      _cube.transform.worldScale = Vec3.new(1.9)
      _cube.transform.worldPosition = Vec3.new(0.1, 2.1, -5.1) * 2

      _cube.addComponent(MeshRender).mesh = _mesh
      _cube.getComponent(MeshRender).subMesh = 0
      _cube.getComponent(MeshRender).albedo = Texture.load("resources/textures/rustediron-streaks_basecolor.png")
      _cube.getComponent(MeshRender).normal = Texture.load("resources/textures/rustediron-streaks_normal.png")
      _cube.getComponent(MeshRender).metallicRoughness = Texture.load("resources/textures/rustediron-streaks_metallic_roughness.png")
      var collider = _cube.addComponent(Collider)
      var pos = _cube.transform.worldPosition
      var rigidBody = _cube.addComponent(RigidBody)
      collider.makeBoxCollider()
      _cube.transform.worldPosition = pos
    }

    update() {
      if (_cube.transform.worldPosition.y < -10) {
        _cube.transform.worldPosition = Vec3.new(0.1, 2.1, -5.1) * 2
      }
    }
}