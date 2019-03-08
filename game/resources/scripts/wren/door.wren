import "Core/Vec2" for Vec2
import "Core/Vec3" for Vec3
import "Core/Vec4" for Vec4

import "Core/Texture" for Texture
import "Core/Shader" for Shader
import "Core/Mesh" for Mesh
import "Core/Console" for Console

import "Core/GameObject" for GameObject

import "Core/Transform" for Transform
import "Core/Camera" for Camera
import "Core/Lod" for Lod
import "Core/RigidBody" for RigidBody
import "Core/WaveSource" for WaveSource
import "Core/Collider" for Collider
import "Core/MonoBehaviour" for MonoBehaviour
import "Core/MeshRender" for MeshRender
import "Core/Physics" for Physics
import "Core/Manifold" for Manifold

import "Core/Input" for Input, Keys, Buttons, Axes
import "Core/Math" for Math

import "Core/Time" for Time
import "Core/Debug" for Debug
import "Core/Sort" for Sort

class Door is MonoBehaviour {
  construct new() { super() }
  static goGet(val) { MonoBehaviour.goGet(val) }
  static goRemove(val) { MonoBehaviour.goRemove(val) }

  onCollisionEnter(other, normal) {
    if (other.name == "player") {
      _door.destroy()
    }
  }

  initialize() {
    // Button.
    gameObject.name         = "button"
    transform.worldScale    = Vec3.new(0.1, 0.5, 0.5) * 2.5
    transform.worldPosition = Vec3.new(-4.75, 0.5, -4.0) * 2.5

    gameObject.addComponent(MeshRender).mesh              = Mesh.generate("cube")
    gameObject.getComponent(MeshRender).subMesh           = 0
    gameObject.getComponent(MeshRender).albedo            = Texture.load("resources/textures/rustediron-streaks_basecolor.png")
    gameObject.getComponent(MeshRender).normal            = Texture.load("resources/textures/rustediron-streaks_normal.png")
    gameObject.getComponent(MeshRender).metallicRoughness = Texture.load("resources/textures/rustediron-streaks_metallic_roughness.png")
    
    var button_collider = gameObject.addComponent(Collider)
    button_collider.makeBoxCollider()

    // Door.
    _door = GameObject.new()

    _door.name                    = "door"
    _door.transform.worldScale    = Vec3.new(0.25, 2.0, 2.0) * 2.5
    _door.transform.worldPosition = Vec3.new(-5.0, 1.0, 0.0) * 2.5

    _door.addComponent(MeshRender).mesh              = Mesh.generate("cube")
    _door.getComponent(MeshRender).subMesh           = 0
    _door.getComponent(MeshRender).albedo            = Texture.load("resources/textures/rustediron-streaks_basecolor.png")
    _door.getComponent(MeshRender).normal            = Texture.load("resources/textures/rustediron-streaks_normal.png")
    _door.getComponent(MeshRender).metallicRoughness = Texture.load("resources/textures/rustediron-streaks_metallic_roughness.png")
  
    var door_collider = _door.addComponent(Collider)
    door_collider.makeBoxCollider()
  }
}
