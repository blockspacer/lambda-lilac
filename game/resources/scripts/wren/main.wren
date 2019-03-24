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

import "Core/Graphics" for Graphics
import "Core/GUI" for GUI
import "Core/Time" for Time
import "Core/File" for File
import "Core/Assert" for Assert

import "Core/MonoBehaviour" for MonoBehaviour

import "Core/PostProcess" for PostProcess
import "Core/Console" for Console
import "Core/Physics" for Physics

import "resources/scripts/wren/post_processor" for PostProcessor
import "resources/scripts/wren/input_controller" for InputController
import "resources/scripts/wren/camera" for FreeLookCamera
import "resources/scripts/wren/lighting" for Lighting
import "resources/scripts/wren/trees" for Trees
import "resources/scripts/wren/item_manager" for ItemManager
import "resources/scripts/wren/door" for Door
import "resources/scripts/wren/physics_layers" for PhysicsLayers

class World {
    construct new() {}

    initialize() {
        _item_manager = GameObject.new().addComponent(ItemManager)

        _post_processer = PostProcessor.new()
        _lighting = Lighting.new()

        _camera = GameObject.new()
				_camera.addComponent(FreeLookCamera)
        
        _door = GameObject.new()
				_door.addComponent(Door)

        // Move the camera so you do not spawn under the ground.
        var new_position = _camera.transform.worldPosition
        new_position.y   = 2.0
        _camera.transform.worldPosition = new_position

        _item_manager.cube(Vec3.new(-11.0, 9.0, 10.0), Vec3.new(2.0))
        _item_manager.cube(Vec3.new(-10.0, 7.0, 10.0), Vec3.new(2.5))
        _item_manager.cube(Vec3.new(-10.0, 9.0,  9.0), Vec3.new(4.0, 0.5, 2.00))
        //_item_manager.spear(Vec3.new(2.0, 9.0, 0.0))

        // Top floor torches.
        _item_manager.torch(Vec3.new( 10.0, 6.0,  12.25), Vec3.new(0.5), Vec3.new(0.0, 0.0 * Math.deg2Rad, 0.0))
        _item_manager.torch(Vec3.new(  0.0, 6.0,  12.25), Vec3.new(0.5), Vec3.new(0.0, 0.0 * Math.deg2Rad, 0.0))
        _item_manager.torch(Vec3.new(-10.0, 6.0,  12.25), Vec3.new(0.5), Vec3.new(0.0, 0.0 * Math.deg2Rad, 0.0))
        _item_manager.torch(Vec3.new( 10.0, 6.0, -12.25), Vec3.new(0.5), Vec3.new(0.0, 180.0 * Math.deg2Rad, 0.0))
        _item_manager.torch(Vec3.new(  0.0, 6.0, -12.25), Vec3.new(0.5), Vec3.new(0.0, 180.0 * Math.deg2Rad, 0.0))
        _item_manager.torch(Vec3.new(-10.0, 6.0, -12.25), Vec3.new(0.5), Vec3.new(0.0, 180.0 * Math.deg2Rad, 0.0))

        // Bottom floor torches.
        _item_manager.torch(Vec3.new( 10.0, 1.0,  12.25), Vec3.new(0.5), Vec3.new(0.0, 0.0 * Math.deg2Rad, 0.0))
        _item_manager.torch(Vec3.new(  0.0, 1.0,  12.25), Vec3.new(0.5), Vec3.new(0.0, 0.0 * Math.deg2Rad, 0.0))
        _item_manager.torch(Vec3.new(-10.0, 1.0,  12.25), Vec3.new(0.5), Vec3.new(0.0, 0.0 * Math.deg2Rad, 0.0))
        _item_manager.torch(Vec3.new( 10.0, 1.0, -12.25), Vec3.new(0.5), Vec3.new(0.0, 180.0 * Math.deg2Rad, 0.0))
        _item_manager.torch(Vec3.new(  0.0, 1.0, -12.25), Vec3.new(0.5), Vec3.new(0.0, 180.0 * Math.deg2Rad, 0.0))
        _item_manager.torch(Vec3.new(-10.0, 1.0, -12.25), Vec3.new(0.5), Vec3.new(0.0, 180.0 * Math.deg2Rad, 0.0))

        // Misc torches.
        _item_manager.torch(Vec3.new(12.25, 1.5, 0.0), Vec3.new(0.5), Vec3.new(0.0, 90.0 * Math.deg2Rad, 0.0))

        {
          var model = GameObject.new()
          model.transform.worldScale = Vec3.new(2.5)
          var mesh = Mesh.load("resources/gltf/lvl/lvl.gltf")
          model.addComponent(MeshRender).attach(mesh)
          var collider = model.addComponent(Collider)
          model.getComponent(MeshRender).makeStaticRecursive()
          collider.makeMeshColliderRecursive(mesh)
          collider.layersRecursive = PhysicsLayers.General | PhysicsLayers.MovingObjects

          new_position.y = 2.0
          _camera.transform.worldPosition = new_position
        }

        GUI.bindCallback("changedSetting(_,_)", this)
        GUI.bindCallback("movedToMenu(_)", this)
    }
    deinitialize() {

    }
    update() {
      GUI.executeJavaScript("updateDebug(%(Time.deltaTime));")
      
      // Update the RSM light.
      //var sin_y_rot = -Math.sin(Math.deg2Rad * _camera.getComponent(FreeLookCamera).rotation.y)
      //var cos_y_rot = -Math.cos(Math.deg2Rad * _camera.getComponent(FreeLookCamera).rotation.y)
      //var offset = Vec3.new(sin_y_rot, 0.0, cos_y_rot).normalized * 12.5
      //var transform = _lighting.rsm.getComponent(Transform)
      //transform.worldPosition = _camera.transform.worldPosition + offset
    }

    fixedUpdate() {
    }

    changedSetting(id, val) {
      if (id == "vsync") {
        Graphics.setVSync(val > 0.0 ? true : false)
      }
      if (id == "render_scale") {
        Graphics.setRenderScale(val)
      }
      if (id == "physics_debug_draw") {
        Physics.debugDrawEnabled = val > 0.0 ? true : false
      }
    }

    movedToMenu(menu) {
      _camera.getComponent(FreeLookCamera).hasInput = menu == "" ? true : false
    }
}
