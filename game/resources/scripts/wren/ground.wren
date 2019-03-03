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
      model.transform.worldScale = Vec3.new(2)
      model.transform.worldPosition = Vec3.new(x - 10, y, z - 10) * 2
      var mesh = Mesh.generate("cube")
      model.addComponent(MeshRender).mesh = mesh
      model.getComponent(MeshRender).subMesh = 0
      model.getComponent(MeshRender).albedo = Texture.load("resources/textures/mossy-ground1-albedo.png")
      model.getComponent(MeshRender).normal = Texture.load("resources/textures/mossy-ground1-normal.png")
      model.getComponent(MeshRender).metallicRoughness = Texture.load("resources/textures/mossy-ground1-metallic-roughness.png")
      var collider = model.addComponent(Collider)
      collider.makeMeshColliderRecursive(mesh)
      _tiles[z][y][x] = model
    }

    initialize() {
      for (z in 0..20) {
        for (x in 0..20) {
          addTile(x, 0, z)
          if (z == 0 || z == 20 || x == 0 || x == 20) {
            addTile(x, 1, z)
          }
        }
      }

    }
}