import "Core/Vec2" for Vec2
import "Core/Vec3" for Vec3

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

import "Core/Input" for Input, Keys, Buttons, Axes
import "Core/Math" for Math

import "Core/Time" for Time

import "resources/scripts/wren/physics_layers" for PhysicsLayers

class ShurikenBehaviour is MonoBehaviour {
  static goGet(val) { MonoBehaviour.goGet(val) }
  static goRemove(val) { MonoBehaviour.goRemove(val) }
  construct new() { super() }

  initialize(position, direction) {
    if(!__mesh) {
      __mesh = Mesh.load("resources/gltf/shuriken.glb")
    }
    _speed = 800.0
    _rotation_speed = Math.deg2Rad * 360.0 * 3.5
    _time = 0.0
    _collided = false
    _direction = direction
    transform.worldPosition = position
    gameObject.tags = [ "shuriken" ]
    
    // Set the scale.
    transform.worldScale = Vec3.new(0.25)
    
    // Add the collider.
    gameObject.addComponent(Collider)
    // Make it a sphere collider.
    gameObject.getComponent(Collider).makeSphereCollider()
    gameObject.getComponent(Collider).layers = PhysicsLayers.MovingObjects
    
    // Add a rigid body.
    gameObject.addComponent(RigidBody)
    // TODO (Hilze): Add.
    //getComponent(RigidBody).velocityConstraints = 0
    // TODO (Hilze): Add.
    gameObject.getComponent(RigidBody).angularConstraints  = 0

    // Create the mesh render.
    _mesh_render = GameObject.new()
    // Set the rotation.
    _mesh_render.transform.worldEuler = Vec3.new(0.0, Math.atan2(direction.x, direction.z), 0.0)
    // Add a mesh render.
    _mesh_render.addComponent(MeshRender)
    // Attach the mesh.
    _mesh_render.getComponent(MeshRender).attach(__mesh)
    // Set the parent.
    _mesh_render.transform.parent = gameObject
    // Set the scale.
    _mesh_render.transform.worldScale = Vec3.new(1.0 / 0.5)
  }

  fixedUpdate() {
    if (!_collided) {
      gameObject.getComponent(RigidBody).velocity = _direction * Time.fixedDeltaTime * _speed + Vec3.new(0.0, 9.81 * Time.fixedDeltaTime, 0.0)
    }

    _time = _time + Time.fixedDeltaTime

    if(_time >= 10.0) {
      gameObject.destroy()
      return
    }
  }
  update() {
    if (!_collided) {
      _mesh_render.transform.rotateEulerWorld(Vec3.new(Time.deltaTime * _rotation_speed, 0.0, 0.0))
    }   
  }

  onCollisionEnter(other, normal) {
    if (!_collided) {

      gameObject.removeComponent(RigidBody)
      gameObject.removeComponent(Collider)
      //_mesh_render.getComponent(MeshRender).makeStaticRecursive()
    }
    _collided = true
  }

  static create(position, direction) {
    var go = GameObject.new()
    go.addComponent(ShurikenBehaviour).initialize(position, direction)
    return go
  }
}