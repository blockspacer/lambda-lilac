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

import "resources/scripts/wren/physics_layers" for PhysicsLayers

class ItemManager is MonoBehaviour {
    construct new()      { super() }
    static goGet(val)    { MonoBehaviour.goGet(val) }
    static goRemove(val) { MonoBehaviour.goRemove(val) }

    initialize() {
        _meshes = []
        _dynamics = []
        
        _meshes.add(Mesh.generate("cube"))
        _meshes.add(Mesh.generate("sphere"))
    }

    cube(position, scale) {
        var c = GameObject.new()

        c.name                    = "cube"
        c.transform.worldScale    = scale
        c.transform.worldPosition = position

        c.addComponent(MeshRender).mesh              = _meshes[0]
        c.getComponent(MeshRender).subMesh           = 0
        c.getComponent(MeshRender).albedo            = Texture.load("resources/textures/woodframe1b-albedo.png")
        c.getComponent(MeshRender).normal            = Texture.load("resources/textures/woodframe1-normal2.png")
        c.getComponent(MeshRender).metallicRoughness = Texture.load("resources/textures/woodframe1-metallic-roughness.png")
       
        var collider  = c.addComponent(Collider)
        var rigidBody = c.addComponent(RigidBody)
        collider.makeBoxCollider()
        collider.layers    = PhysicsLayers.General | PhysicsLayers.MovingObjects
        rigidBody.mass     = 1.0
        rigidBody.friction = 2.0

        _dynamics.add(c)

        return c
    }

    spear(position) {
        var c = GameObject.new()

        c.name                    = "spear"
        c.transform.worldScale    = Vec3.new(0.25, 0.25, 4.0)
        c.transform.worldPosition = position

        c.addComponent(MeshRender).mesh              = _meshes[0]
        c.getComponent(MeshRender).subMesh           = 0
        c.getComponent(MeshRender).albedo            = Texture.load("resources/textures/mossy-ground1-albedo.png")
        c.getComponent(MeshRender).normal            = Texture.load("resources/textures/mossy-ground1-normal.png")
        c.getComponent(MeshRender).metallicRoughness = Texture.load("resources/textures/mossy-ground1-metallic-roughness.png")
        
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
        for(c in _dynamics) {
            if (c.transform.worldPosition.y < -10) {
                c.transform.worldPosition = Vec3.new(0.0, 2.0, -0.0) * 2
                c.getComponent(RigidBody).velocity = Vec3.new(0.0)
            }
        }
    }
}