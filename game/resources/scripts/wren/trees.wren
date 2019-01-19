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
import "resources/scripts/wren/ground" for Ground

class Trees {
    construct new(ground) {
        _enabled = true
        _trees = []

        initialize(ground)
    }
    enabled=(v) { _enabled = v }


    initialize(ground) {
        if (!_enabled) return

        // Get all of the models.
        var tree_mesh_01 = Mesh.load("resources/gltf/tree_01.glb")
        var tree_mesh_02 = Mesh.load("resources/gltf/tree_02.glb")
        var tree_mesh_03 = Mesh.load("resources/gltf/tree_03.glb")
        var tree_mesh_04 = Mesh.load("resources/gltf/tree_04.glb")
 
        var bush_mesh_01 = Mesh.load("resources/gltf/bush_01.glb")
        var bush_mesh_02 = Mesh.load("resources/gltf/bush_02.glb")
        var bush_mesh_03 = Mesh.load("resources/gltf/bush_03.glb")

        // Spawn all of the trees.
        var scale = 10.0
        var rarity = 0.33
        var num_trees = 5000
        var num_x = Math.round(Math.sqrt(num_trees)) - 3
        var num_z = num_trees / num_x

        var actual_count = 0

        for (x in (-num_x / 2)..(num_x / 2)) { 
            if (x != 0 && x != -1) {

                for (z in (-num_z / 2)..(num_z / 2)) { 
                    var position = Vec3.new((x + Math.random(0.2, 0.8)) * scale, 0.0, (z + Math.random(0.2, 0.8)) * scale)
                
                    position.y = ground.heightOnPosition(position)

                    if (Math.random() < rarity && position.y > ground.waterHeight) {
                        actual_count = actual_count + 1
                        var tree = GameObject.new()
                        var transform = tree.transform
                        var mesh_render = tree.addComponent(MeshRender)
                        
                        transform.localPosition = position
                        var selected_mesh = Math.floor(Math.random() * (4.0 * 2.0 + 3.0))
                        selected_mesh = (selected_mesh < 8) ? (selected_mesh / 2) : (selected_mesh - 4)

                        if (selected_mesh == 0) {
                          mesh_render.attach(tree_mesh_01)
                          tree.addComponent(Collider).makeMeshColliderRecursive(tree_mesh_01)
                        }
                        if (selected_mesh == 1) {
                          mesh_render.attach(tree_mesh_02)
                          tree.addComponent(Collider).makeMeshColliderRecursive(tree_mesh_02)
                        }
                        if (selected_mesh == 2) {
                          mesh_render.attach(tree_mesh_03)
                          tree.addComponent(Collider).makeMeshColliderRecursive(tree_mesh_03)
                        }
                        if (selected_mesh == 3) {
                          mesh_render.attach(tree_mesh_04)
                          tree.addComponent(Collider).makeMeshColliderRecursive(tree_mesh_04)
                        }
                        if (selected_mesh == 4) {
                          mesh_render.attach(bush_mesh_01)
                          tree.addComponent(Collider).makeMeshColliderRecursive(bush_mesh_01)
                        }
                        if (selected_mesh == 5) {
                          mesh_render.attach(bush_mesh_02)
                          tree.addComponent(Collider).makeMeshColliderRecursive(bush_mesh_02)
                        }
                        if (selected_mesh == 6) {
                          mesh_render.attach(bush_mesh_03)
                          tree.addComponent(Collider).makeMeshColliderRecursive(bush_mesh_03)
                        }
                        
                        mesh_render.makeStaticRecursive()
                        _trees.add(tree)
                    } 
                }
            }
        }

        Console.info("[TREE]: Hard maximum number of trees: %(num_trees)")
        Console.info("[TREE]: Estimated maximum number of trees: %(num_x * num_z)")
        Console.info("[TREE]: Actual number of trees: %(actual_count)")
    }
}