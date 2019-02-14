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

import "resources/scripts/wren/post_processor" for PostProcessor
import "resources/scripts/wren/input_controller" for InputController
import "resources/scripts/wren/camera" for FreeLookCamera
import "resources/scripts/wren/lighting" for Lighting
import "resources/scripts/wren/trees" for Trees
import "resources/scripts/wren/ground" for Ground

class World {
    construct new() {}
    initialize() {
        _ground = Ground.new()
        _trees = Trees.new(_ground)
        _ground.disable()
        _trees.enabled = false
        _ground.initialize()
        _trees.initialize()

        PostProcess.setShaderVariableFloat1("water_height", _ground.waterHeight)

        _post_processer = PostProcessor.new()
        _lighting = Lighting.new()

        _camera = GameObject.new()
				_camera.addComponent(FreeLookCamera)

        // Move the camera so you do not spawn under the ground.
        var new_position = _camera.transform.worldPosition
        new_position.y = _ground.heightOnPosition(new_position) + 2.0
        _camera.transform.worldPosition = new_position

        {
          var model = GameObject.new()
          model.transform.worldScale = Vec3.new(0.5)
          var mesh = Mesh.load("resources/gltf/map.glb")
          model.addComponent(MeshRender).attach(mesh)
          var collider = model.addComponent(Collider)
          collider.makeMeshColliderRecursive(mesh)

          new_position.y = _ground.heightOnPosition(new_position) + 10.0
          _camera.transform.worldPosition = new_position

          // {
          //   var model2 = GameObject.new()
          //   model2.transform.worldScale = Vec3.new(10.0)
          //   var mesh2 = Mesh.load("resources/gltf/camera.glb")
          //   model2.addComponent(MeshRender).attach(mesh2)
          // }
        }

        _init_once = true
    }
    deinitialize() {

    }
    update() {
        if (_init_once) {
            _init_once = false
            //PostProcess.addShaderPass("apply_gui", Shader.load("resources/shaders/apply_gui.fx"), [ "post_process_buffer", "gui" ], [ "post_process_buffer" ])
        }
        
        // Update the RSM light.
        var sin_y_rot = -Math.sin(Math.deg2Rad * _camera.getComponent(FreeLookCamera).rotation.y)
        var cos_y_rot = -Math.cos(Math.deg2Rad * _camera.getComponent(FreeLookCamera).rotation.y)
        var offset = Vec3.new(sin_y_rot, 0.0, cos_y_rot).normalized * 25.0
        var transform = _lighting.rsm.getComponent(Transform)
        transform.worldPosition = _camera.transform.worldPosition + offset
    }
    fixedUpdate() {
    }
}
