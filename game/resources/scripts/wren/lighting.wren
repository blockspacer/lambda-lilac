import "Core/Vec2" for Vec2
import "Core/Vec3" for Vec3
import "Core/Quat" for Quat

import "Core/Texture" for Texture, TextureFormat
import "Core/Shader" for Shader
import "Core/Wave" for Wave
import "Core/Mesh" for Mesh

import "Core/GameObject" for GameObject

import "Core/Transform" for Transform
import "Core/Camera" for Camera
import "Core/Lod" for Lod
import "Core/RigidBody" for RigidBody
import "Core/WaveSource" for WaveSource
import "Core/Collider" for Collider
import "Core/Light" for Light, LightTypes, ShadowTypes

import "Core/Input" for Input, Keys, Buttons, Axes
import "Core/Math" for Math

import "Core/Time" for Time

import "Core/PostProcess" for PostProcess

class Lighting {
  construct new() {
    //createSun()
    createReflectiveShadowMap()
    //createFlashLight()
  }

  sunFront   { _sun_front  }
  sunBack    { _sun_back   }
  flashlight { _flashlight }
  rsm        { _rsm        }

  createSun() {
    //--------------------SUN-FRONT--------------------

    // Create all required things.
    PostProcess.addRenderTarget("sun_front_rt1", 512.0  / 4.0, 512.0  / 4.0, TextureFormat.R32G32B32A32)
    PostProcess.addRenderTarget("sun_front_rt2", 1024.0 / 4.0, 1024.0 / 4.0, TextureFormat.R32G32B32A32)
    PostProcess.addRenderTarget("sun_front_rt3", 2048.0 / 4.0, 2048.0 / 4.0, TextureFormat.R32G32B32A32)
    
    _sun_front = GameObject.new()

    // Add a cascaded light.
    var light_sun_front = _sun_front.addComponent(Light)
    light_sun_front.type = LightTypes.Cascade

    //light_sun_front.MakeRSM()
    light_sun_front.lightColour = Vec3.new(1.0, 1.0, 1.0)
    light_sun_front.ambientColour = Vec3.new(0.2, 0.2, 0.2)
    light_sun_front.lightIntensity = 7.0
    light_sun_front.shadowType = ShadowTypes.Dynamic
    light_sun_front.dynamicFrequency = 25.0
    light_sun_front.renderTargets = [ "sun_front_rt1", "sun_front_rt2", "sun_front_rt3" ]

    //--------------------SUN-BACK--------------------

    return

    // Create all required things.
    _sun_back = GameObject.new()

    // Add a directional light.
    var light_sun_back = _sun_back.addComponent(Light)
    light_sun_back.type = LightTypes.Directional

    light_sun_back.lightColour = Vec3.new(0.2, 0.2, 0.25)

    light_sun_front.enabled = false
    light_sun_back.enabled  = false
  }

  createFlashLight() {
    //--------------------FLASH-LIGHT--------------------

    // Create all required things.
    PostProcess.addRenderTarget("flash_light_rt", 512.0, 512.0, TextureFormat.R32G32B32A32)

    // Create all required things.
    _flashlight = GameObject.new()

    // Add a spot light.
    var light_flashlight = _flashlight.addComponent(Light)
    light_flashlight.type = LightTypes.Directional


    // Add a transform.
    var trans_flashlight = _flashlight.getComponent(Transform)
    trans_flashlight.worldEuler = Vec3.new(0.0, Math.deg2Rad * 90.0, 0.0)
    trans_flashlight.worldPosition = Vec3.new(7.0, 2.0, 0.0)
    
    //light_flashlight.MakeRSM()
    light_flashlight.renderTargets = [ "rsm_shad", "rsm_posi", "rsm_norm", "rsm_flux" ]
    light_flashlight.lightColour = Vec3.new(1.0, 1.0, 1.0)
    light_flashlight.intensity = 20.0
    light_flashlight.depth = 100.0
    light_flashlight.innerCutOff = Math.deg2Rad * 30.0
    light_flashlight.outerCutOff = Math.deg2Rad * 50.0
    light_flashlight.shadowType = ShadowTypes.Dynamic
    //light_flashlight.renderTargets = [ "flash_light_rt" ]
    light_flashlight.texture = Texture.load("resources/textures/flashlight.png")
    light_flashlight.enabled = false


    // Add a sound source.
    var wave_source = _flashlight.addComponent(WaveSource)
    wave_source.buffer = Wave.load("resources/waves/light_switch.wav")
    wave_source.relativeToListener = false
  }

  createReflectiveShadowMap() {
    _rsm = GameObject.new()

    var trans_rsm = _rsm.getComponent(Transform)
    trans_rsm.worldRotation = Math.lookRotation(Vec3.new(0.0, 1.0, 1.0).normalized, Vec3.new(0.0, 1.0, 0.0))

    PostProcess.addRenderTarget("rsm_shad", 2048.0, 2048.0, TextureFormat.R16G16B16A16)
    PostProcess.addRenderTarget("rsm_posi", 2048.0, 2048.0, TextureFormat.R16G16B16A16)
    PostProcess.addRenderTarget("rsm_norm", 2048.0, 2048.0, TextureFormat.R8G8B8A8)
    PostProcess.addRenderTarget("rsm_flux", 2048.0, 2048.0, TextureFormat.R16G16B16A16)

    // Add a RSM light.
    var light_rsm = _rsm.addComponent(Light)
    light_rsm.type = LightTypes.Directional
    //light_rsm.isRSM = true

    light_rsm.lightColour = Vec3.new(1.0, 1.0, 1.0)
    light_rsm.lightIntensity = 7.0
    light_rsm.shadowType = ShadowTypes.Dynamic
    //light_rsm.renderTargets = [ "rsm_shad", "rsm_posi", "rsm_norm", "rsm_flux" ]
    light_rsm.renderTargets = [ "rsm_shad" ]
    light_rsm.depth = 200.0
    light_rsm.size = 100.0
    //light_rsm.enabled = false
  }
}