import "Core" for Vec2, Vec3, Vec4
import "Core" for Texture, Shader, Mesh
import "Core" for GameObject, Transform, Camera, MeshRender, Lod, RigidBody, WaveSource, Collider
import "Core" for Input, Keys, Buttons, Axes
import "Core" for Math, Time, File, Assert
import "Core" for MonoBehaviour, PostProcess, Console
import "Core" for Noise, NoiseInterpolation
import "Core" for Light, LightTypes, ShadowTypes

import "resources/scripts/wren/physics_layers" for PhysicsLayers

class ItemManager is MonoBehaviour {
  construct new()      { super() }
  static goGet(val)    { MonoBehaviour.goGet(val) }
  static goRemove(val) { MonoBehaviour.goRemove(val) }

  initialize() {
    _meshes = []
    _dynamics = []
    _statics = []
    _lights = []
    _light_last_color = Vec3.new(1.0, 1.0, 1.0)
    _light_new_color = Vec3.new(1.0, 1.0, 1.0)
    _light_last_distance = 2.0
    _light_new_distance = 2.0
    _light_time = 0.0
    _light_switch = 0.2
      
    _meshes.add(Mesh.generate("cube"))
    _meshes.add(Mesh.generate("sphere"))
    _meshes.add(Mesh.load("resources/gltf/torch/torch.gltf"))

    _total_time = 0.0
    _multi_moving = GameObject.new()

    _multi_moving.name                    = "multi_moving"
    _multi_moving.transform.worldPosition = Vec3.new(0.0, 10.0, 0.0)
    var light = _multi_moving.addComponent(Light)
    light.type = LightTypes.Point

    light.lightColour = Vec3.new(255, 200, 200) / 255
    light.lightIntensity = 2.5
    light.depth = 50.0
    light.shadowType = ShadowTypes.GenerateOnce
    light.shadowMapSizePx = 256.0
  }

  cube(position, scale) {
    var c = GameObject.new()

    c.name                    = "cube"
    c.transform.worldScale    = scale
    c.transform.worldPosition = position

    c.addComponent(MeshRender).mesh     = _meshes[0]
    c.getComponent(MeshRender).subMesh  = 0
    c.getComponent(MeshRender).albedo   = Texture.load("resources/textures/wood/FloorMahogany_alb.jpg")
    c.getComponent(MeshRender).normal   = Texture.load("resources/textures/wood/FloorMahogany_nrm.jpg")
    c.getComponent(MeshRender).DMRA     = Texture.load("resources/textures/wood/FloorMahogany_dmra.png")
    
    var collider  = c.addComponent(Collider)
    var rigidBody = c.addComponent(RigidBody)
    collider.makeBoxCollider()
    collider.layers    = PhysicsLayers.General | PhysicsLayers.MovingObjects
    rigidBody.mass     = 1.0
    rigidBody.friction = 2.0

    _dynamics.add(c)

    return c
  }

  torch(position, scale, euler) {
    var c = GameObject.new()

    c.name                    = "torch"
    c.transform.worldScale    = scale
    c.transform.worldPosition = position
    c.transform.worldEuler    = euler

    c.addComponent(MeshRender).attach(_meshes[2])
    c.getComponent(MeshRender).makeStaticRecursive()

    var l = GameObject.new()
    l.transform.parent = c
    l.transform.localPosition = Vec3.new(0.0, 2.5, -1.0)

    // Add a point light.
    var light = l.addComponent(Light)
    light.type = LightTypes.Point

    light.lightColour = Vec3.new(0.0, 1.0, 0.25)
    light.lightIntensity = 40.0
    light.depth = 2.0
    light.shadowType = ShadowTypes.GenerateOnce
    light.shadowMapSizePx = 128.0

    _statics.add(c)
    _lights.add(light)

    return c
  }

  spear(position) {
    var c = GameObject.new()

    c.name                    = "spear"
    c.transform.worldScale    = Vec3.new(0.25, 0.25, 4.0)
    c.transform.worldPosition = position

    c.addComponent(MeshRender).mesh    = _meshes[0]
    c.getComponent(MeshRender).subMesh = 0
    c.getComponent(MeshRender).albedo  = Texture.load("resources/textures/FloorMahogany_alb.jpg")
    c.getComponent(MeshRender).normal  = Texture.load("resources/textures/FloorMahogany_nrm.jpg")
    c.getComponent(MeshRender).DMRA    = Texture.load("resources/textures/FloorMahogany_dmra.png")
    
    var collider  = c.addComponent(Collider)
    var rigidBody = c.addComponent(RigidBody)
    collider.makeBoxCollider()
    collider.layers    = PhysicsLayers.General | PhysicsLayers.MovingObjects
    rigidBody.mass     = 1.0
    rigidBody.friction = 2.0

    _dynamics.add(c)

    return c
  }

  fixedUpdate() {
    for (c in _dynamics) {
      if (c.transform.worldPosition.y < -10) {
        c.transform.worldPosition = Vec3.new(0.0, 2.0, -0.0) * 2
        c.getComponent(RigidBody).velocity = Vec3.new(0.0)
      }
    }

    _light_time = _light_time + Time.fixedDeltaTime
    if (_light_time > _light_switch) {
      _light_time = _light_time - _light_switch
      _light_last_color = _light_new_color
      _light_new_color = Vec3.new(
        Math.random(0.75, 1.0),
        Math.random(0.5, 1.0),
        Math.random(0.0, 0.25)
      )
      _light_last_distance = _light_new_distance
      _light_new_distance = Math.random(4.0, 5.0)
    }

    var d = _light_time / _light_switch

    var color = Vec3.new(
      Math.lerp(_light_last_color.x, _light_new_color.x, d),
      Math.lerp(_light_last_color.y, _light_new_color.y, d),
      Math.lerp(_light_last_color.z, _light_new_color.z, d)
    )
    var distance = Math.lerp(_light_last_distance, _light_new_distance, d)

    for (l in _lights) {
      l.lightColour = color
      l.depth = distance
    }
  }
}