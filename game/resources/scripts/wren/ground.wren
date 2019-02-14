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
        _enabled = GroundEnabledType.Enabled
        _ground = GameObject.new()
        _water = GameObject.new()
        _mesh = Mesh.create()
        _offset = Vec3.new(0.0)
        _scale = Vec3.new(5.0, 50.0, 5.0)
        _size = Vec2.new(250, 250)
        _water_height = -10.0
    }

    enable() { _enabled = GroundEnabledType.Enabled }
    disable() { _enabled = GroundEnabledType.Disabled }
    zeroHeight() { _enabled = GroundEnabledType.ZeroHeight }
    waterHeight { enabled == GroundEnabledType.Enabled ? _water_height : 0.0 }
    enabled { _enabled == GroundEnabledType.Enabled ? true : false }

    heightOnTile(tile) {
        var idx = Math.round(tile.x + tile.y * _size.x)
        if (idx < 0 || idx >= _heights.count) {
            return 0.0
        }
        return _heights[idx]
    }

    heightOnPosition(position) {
        if (_enabled != GroundEnabledType.Enabled) return 0.0

        var t = (position - _offset) / _scale
        var tile = Vec2.new(t.x, t.z)

        if (tile.x < 0 || tile.y < 0 || tile.x >= _size.x || tile.y >= _size.y) {
            return 0.0
        }

        var bl = heightOnTile(Vec2.new(Math.floor(tile.x), Math.floor(tile.y)))
        var tl = heightOnTile(Vec2.new(Math.floor(tile.x), Math.floor(tile.y) + 1))
        var br = heightOnTile(Vec2.new(Math.floor(tile.x) + 1, Math.floor(tile.y)))
        var tr = heightOnTile(Vec2.new(Math.floor(tile.x) + 1, Math.floor(tile.y) + 1))

        var a = Math.lerp(bl, br, tile.x - Math.floor(tile.x))
        var b = Math.lerp(tl, tr, tile.x - Math.floor(tile.x))
        var c = Math.lerp(a,  b,  tile.y - Math.floor(tile.y))
        return c
    }

    normalOfQuad(a, b, c, d) {
        return (a.cross(b) + b.cross(c) + c.cross(d) + d.cross(a)).normalized
    }

    initialize() {
        if (_enabled == GroundEnabledType.Disabled) return

        var noise_big = Noise.new()
        noise_big.frequency = 0.01
        noise_big.seed = 256
        noise_big.interpolation = NoiseInterpolation.Quintic
        
        var noise_small = Noise.new()
        noise_small.frequency = 0.1
        noise_small.seed = noise_big.seed * 2
        noise_small.interpolation = NoiseInterpolation.Quintic

        // Generate positions
        var positions = List.filled(_size.x * _size.y, Vec3.new(0.0))
        _heights = List.filled(positions.count, 0.0)
        var tex_coords = List.filled(positions.count, Vec2.new(0.0))

        for (h in 0..._size.y) {
            for (w in 0..._size.x) {
                var idx = h * _size.x + w
                var position = Vec2.new(w, h)
                var new_height = noise_big.getPerlin(position * Vec2.new(_scale.x, _scale.z))
                new_height = new_height + noise_small.getPerlin(position * Vec2.new(_scale.x, _scale.z)) / _scale.y
                positions[idx] = Vec3.new(position.x, new_height, position.y) * _scale
                
                if (enabled == GroundEnabledType.ZeroHeight) {
                    positions[idx].y = 0.0
                }
                _heights[idx] = positions[idx].y
            
                tex_coords[idx] = Vec2.new(w % 2.0, h % 2.0)
            }
        }

        var normals = List.filled(positions.count, Vec3.new(0.0))
        var indices = List.filled(positions.count * 6, 0)

        for (h in 0...(_size.y - 1)) {
            for (w in 0...(_size.x - 1)) {
                var idx_a = (h + 0) * _size.x + (w + 0)
                var idx_b = (h + 0) * _size.x + (w + 1)
                var idx_c = (h + 1) * _size.x + (w + 0)
                var idx_d = (h + 1) * _size.x + (w + 1)

                var a = positions[idx_a]
                var b = positions[idx_b]
                var c = positions[idx_c]
                var d = positions[idx_d]

                normals[idx_a] = Vec3.new(-1.0, -1.0, -1.0) * (d - a).cross(c - b)

                indices[idx_a * 6 + 0] = idx_c
                indices[idx_a * 6 + 1] = idx_b
                indices[idx_a * 6 + 2] = idx_a
                indices[idx_a * 6 + 3] = idx_c
                indices[idx_a * 6 + 4] = idx_d
                indices[idx_a * 6 + 5] = idx_b
            }
        }

        for (h in 0..._size.y) {
            normals[(h) * _size.x + (_size.x - 1)] = Vec3.new(0.0, 1.0, 0.0)
        }
        for (w in 0..._size.x) {
            normals[(_size.y - 1) * _size.x + (w)] = Vec3.new(0.0, 1.0, 0.0)
        }

        _mesh.positions = positions
        _mesh.normals   = normals
        _mesh.texCoords = tex_coords
        _mesh.indices   = indices
        _mesh.recalculateTangents()

        _offset = Vec3.new(-1.0, -1.0, -1.0) *   Vec3.new(_size.x * 0.5, 0.0, _size.y * 0.5) * _scale
        
        var albedo_texture = Texture.load("resources/textures/mossy-ground1-albedo.png")
        var normal_texture = Texture.load("resources/textures/mossy-ground1-normal.png")

        var transform = _ground.transform
        var mesh_render = _ground.addComponent(MeshRender)
        
        mesh_render.mesh = _mesh
        mesh_render.albedo = albedo_texture
        mesh_render.normal = normal_texture
        transform.localPosition = _offset
        mesh_render.makeStaticRecursive()
        
        var collider = _ground.addComponent(Collider)
        collider.makeMeshColliderRecursive(_mesh)

        createWater()
    }

    createWater() {
        if (_enabled == GroundEnabledType.Disabled) return

        var water_mesh = Mesh.create()
        var water_positions = []
        var water_normals = []
        var water_tex_coords = []
        var water_indices = []

        var size = _size * Vec2.new(_scale.x, _scale.z)

        water_positions.add(Vec3.new(0.0, _water_height, size.y))
        water_positions.add(Vec3.new(size.x, _water_height, size.y))
        water_positions.add(Vec3.new(size.x, _water_height, 0.0))
        water_positions.add(Vec3.new(0.0, _water_height, 0.0))
        
        water_normals.add(Vec3.new(0.0, 1.0, 0.0))
        water_normals.add(Vec3.new(0.0, 1.0, 0.0))
        water_normals.add(Vec3.new(0.0, 1.0, 0.0))
        water_normals.add(Vec3.new(0.0, 1.0, 0.0))

        water_tex_coords.add(Vec2.new(0.0, 0.0))
        water_tex_coords.add(Vec2.new(size.x, 0.0))
        water_tex_coords.add(Vec2.new(size.x, size.y))
        water_tex_coords.add(Vec2.new(0.0, size.y))

        water_indices.add(0)
        water_indices.add(1)
        water_indices.add(3)
        water_indices.add(1)
        water_indices.add(2)
        water_indices.add(3)

        water_mesh.positions = water_positions
        water_mesh.normals   = water_normals
        water_mesh.texCoords = water_tex_coords
        water_mesh.indices   = water_indices
        water_mesh.recalculateTangents()

        var mesh_render = _water.addComponent(MeshRender)
        var transform = _water.transform

        var albedo_texture = Texture.load("resources/textures/Water-0326-diffuse.png")
        var normal_texture = Texture.load("resources/textures/Water-0326-normal.png")

        mesh_render.mesh = water_mesh
        mesh_render.albedo = albedo_texture
        mesh_render.normal = normal_texture
        transform.localPosition = _offset
        mesh_render.makeStaticRecursive()
    }
}