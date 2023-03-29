import "Core" for Vec2, Vec3, Quat
import "Core" for Texture, TextureFormat, Shader, Wave, Mesh
import "Core" for GameObject, Transform, Camera, Lod, RigidBody, WaveSource, Collider
import "Core" for Light, LightTypes, ShadowTypes
import "Core" for Input, Keys, Buttons, Axes
import "Core" for Math, Time, PostProcess

import "resources/scripts/wren/ini" for Ini

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
    light_sun_front.shadowMapSizePx = 1024.0

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
    _flashlight = GameObject.new()

    // Add a spot light.
    var light_flashlight = _flashlight.addComponent(Light)
    light_flashlight.type = LightTypes.Directional


    // Add a transform.
    var trans_flashlight = _flashlight.getComponent(Transform)
    trans_flashlight.worldEuler = Vec3.new(0.0, Math.deg2Rad * 90.0, 0.0)
    trans_flashlight.worldPosition = Vec3.new(7.0, 2.0, 0.0)
    
    //light_flashlight.MakeRSM()
    light_flashlight.shadowMapSizePx = 512.0
    light_flashlight.lightColour = Vec3.new(1.0, 1.0, 1.0)
    light_flashlight.intensity = 20.0
    light_flashlight.depth = 100.0
    light_flashlight.innerCutOff = Math.deg2Rad * 30.0
    light_flashlight.outerCutOff = Math.deg2Rad * 50.0
    light_flashlight.shadowType = ShadowTypes.Dynamic
    light_flashlight.enabled = false


    // Add a sound source.
    var wave_source = _flashlight.addComponent(WaveSource)
    wave_source.buffer = Wave.load("resources/waves/light_switch.wav")
    wave_source.relativeToListener = false
  }

  createReflectiveShadowMap() {
    _rsm = GameObject.new()

    var size = Ini.new("resources/settings.ini")["Lighting", "ShadowMapSize"]    

    var trans_rsm = _rsm.getComponent(Transform)
    trans_rsm.worldRotation = Math.lookRotation(Vec3.new(0.3, 1.0, 0.2).normalized, Vec3.new(0.0, 1.0, 0.0))

    // Add a RSM light.
    var light_rsm = _rsm.addComponent(Light)
    light_rsm.type = LightTypes.Directional
    //light_rsm.isRSM = true

    light_rsm.lightColour = Vec3.new(1.0, 1.0, 1.0)
    light_rsm.lightIntensity = 3.0
    light_rsm.shadowType = ShadowTypes.Dynamic
    light_rsm.shadowMapSizePx = size
    light_rsm.depth = 500.0
    light_rsm.size = 250.0
    //light_rsm.enabled = false
  }
}
