import "Core" for Vec2, Vec3, Vec4
import "Core" for Texture, Shader, Mesh, Console
import "Core" for GameObject, Transform, Lod, RigidBody, WaveSource, Collider, MonoBehaviour, MeshRender, Physics, Manifold
import "Core" for Input, Keys, Buttons, Axes
import "Core" for Math, Time, Debug, Sort

import "resources/scripts/wren/physics_layers" for PhysicsLayers
import "resources/scripts/wren/meshes" for Meshes

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
    transform.worldPosition = Vec3.new(-4.75, 2.5, -4.1) * 2.5

    gameObject.addComponent(MeshRender).mesh     = Meshes.cube
    gameObject.getComponent(MeshRender).subMesh  = 0
    gameObject.getComponent(MeshRender).albedo   = Texture.load("resources/textures/wood/FloorMahogany_alb.jpg")
    gameObject.getComponent(MeshRender).normal   = Texture.load("resources/textures/wood/FloorMahogany_nrm.jpg")
    gameObject.getComponent(MeshRender).DMRA     = Texture.load("resources/textures/wood/FloorMahogany_dmra.png")
    
    var button_collider = gameObject.addComponent(Collider)
    button_collider.makeBoxCollider()
    button_collider.layers = PhysicsLayers.General | PhysicsLayers.MovingObjects

    // Door.
    _door = GameObject.new()

    _door.name                    = "door"
    _door.transform.worldScale    = Vec3.new(0.25, 2.0, 2.0) * 2.5
    _door.transform.worldPosition = Vec3.new(-5.0, 1.0, 0.0) * 2.5

    _door.addComponent(MeshRender).mesh    = Meshes.cube
    _door.getComponent(MeshRender).subMesh = 0
    _door.getComponent(MeshRender).albedo  = Texture.load("resources/textures/wood/FloorMahogany_alb.jpg")
    _door.getComponent(MeshRender).normal  = Texture.load("resources/textures/wood/FloorMahogany_nrm.jpg")
    _door.getComponent(MeshRender).DMRA    = Texture.load("resources/textures/wood/FloorMahogany_dmra.png")
  
    var door_collider = _door.addComponent(Collider)
    door_collider.makeBoxCollider()
    door_collider.layers = PhysicsLayers.General | PhysicsLayers.MovingObjects
  }
}
