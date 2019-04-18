import "Core/Vec2" for Vec2
import "Core/Vec3" for Vec3
import "Core/Vec4" for Vec4

import "Core/Texture" for Texture, TextureFormat
import "Core/Shader" for Shader
import "Core/Mesh" for Mesh

import "Core/Graphics" for Graphics
import "Core/GUI" for GUI
import "Core/PostProcess" for PostProcess
import "Core/Console" for Console

import "resources/scripts/wren/ini" for Ini

class PostProcessor {
  flipFlopPostProcess() {
    if (_post_process_output == "post_process_buffer") {
      _post_process_output = "post_process_temp"
    } else {
      _post_process_output = "post_process_buffer"
    }
    return _post_process_output
  }
  flipFlopPosition() {
    if (_position_output == "position") {
      _position_output = "position_temp"
    } else {
      _position_output = "position"
    }
    return _position_output
  }
  flipFlopBloom() {
    if (_bloom_output == "bloom") {
      _bloom_output = "bloom_temp"
    } else {
      _bloom_output = "bloom"
    }
    return _bloom_output
  }
  flipFlopDof() {
    if (_dof_output == "dof") {
      _dof_output = "dof_temp"
    } else {
      _dof_output = "dof"
    }
    return _dof_output
  }
  flipFlopDofPos() {
    if (_dof_pos_output == "dof_pos") {
      _dof_pos_output = "dof_pos_temp"
    } else {
      _dof_pos_output = "dof_pos"
    }
    return _dof_pos_output
  }
  construct new() {
    // Load the .INI file.
    var ini_reader = Ini.new("resources/settings.ini")
    
    // Create all required render targets.
    PostProcess.addRenderTarget("albedo",              1.0, TextureFormat.R8G8B8A8)
    PostProcess.addRenderTarget("position",            1.0, TextureFormat.R32G32B32A32)
    PostProcess.addRenderTarget("position_temp",       1.0, TextureFormat.R32G32B32A32)
    PostProcess.addRenderTarget("normal",              1.0, TextureFormat.R8G8B8A8)
    PostProcess.addRenderTarget("metallic_roughness",  1.0, TextureFormat.R8G8B8A8)
    PostProcess.addRenderTarget("depth_buffer",        1.0, TextureFormat.D32)
    PostProcess.addRenderTarget("post_process_buffer", 1.0, TextureFormat.R16G16B16A16)
    PostProcess.addRenderTarget("post_process_temp",   1.0, TextureFormat.R16G16B16A16)
    _post_process_output = "post_process_buffer"
    _position_output = "position"
    
    // Add and generate the environment maps.
    PostProcess.addRenderTarget("brdf_lut", Texture.load("resources/textures/lighting/ibl_brdf_lut.png"))
    PostProcess.addRenderTarget("environment_map", Texture.load("resources/textures/hdr/Serpentine_Valley_3k.hdr"))
    PostProcess.irradianceConvolution("environment_map", "irradiance_map")
    PostProcess.hammerhead("environment_map", "prefiltered")

    // Create all passes.
    copyAlbedoToPostProcessBuffer()
    shadowMapping(
      ini_reader["Lighting", "Enabled"]
    )
    ssao(
      ini_reader["SSAO", "Strength"], 
      Vec2.new(ini_reader["SSAO", "BlurScaleX"], ini_reader["SSAO", "BlurScaleY"]), 
      Vec2.new(ini_reader["SSAO", "BlurPassesX"], ini_reader["SSAO", "BlurPassesY"]), 
      ini_reader["SSAO", "TargetScale"]
    )
    applyLighting(
      ini_reader["Lighting", "Enabled"]
    )
    skyDome(
      ini_reader["Skydome", "Enabled"]
    )
    bloom(
      ini_reader["Bloom", "Enabled"], 
      Vec2.new(ini_reader["Bloom", "BlurScaleX"], ini_reader["Bloom", "BlurScaleY"]), 
      Vec2.new(ini_reader["Bloom", "BlurPassesX"], ini_reader["Bloom", "BlurPassesY"]), 
      ini_reader["Bloom", "TargetScale"]
    )
    toneMapping(
      ini_reader["ToneMapping", "Enabled"]
    )
    fxaa(
      ini_reader["FXAA", "Enabled"]
    )
    dof(
      ini_reader["DOF", "Enabled"],
      Vec2.new(ini_reader["DOF", "BlurScaleX"], ini_reader["DOF", "BlurScaleY"]),
      Vec2.new(ini_reader["DOF", "BlurPassesX"], ini_reader["DOF", "BlurPassesY"]),
      ini_reader["DOF", "TargetScale"]
    )
    water(
      ini_reader["Water", "Enabled"]
    )
    ssr(
      ini_reader["SSR", "Enabled"]
    )
    gui(
      ini_reader["GUI", "Enabled"]
    )

    // Set the final pass.
    PostProcess.setFinalRenderTarget(_post_process_output)
  }

  copyAlbedoToPostProcessBuffer() {
    PostProcess.addShaderPass("copy_albedo_to_post_process_buffer", Shader.load("resources/shaders/copy.fx"), [ "albedo" ], [ _post_process_output ])
  }

  shadowMapping(enabled) {
    if (!enabled) return

    // Get all shared shaders.
    var generate    = "resources/shaders/shadow_mapping/generate.fx"
    var modify      = "resources/shaders/shadow_mapping/blur_7x1.fx"
    var modifyCount = 2
    var apply       = "resources/shaders/shadow_mapping/shadow_mapping.fx"
    var shadowType  = "ESM"

    // Set all lighting shaders.
    Graphics.setLightShaders(generate, modify, modifyCount, apply, shadowType)
  }
  applyLighting(enabled) {
    PostProcess.addShaderPass("apply_lighting", Shader.load("resources/shaders/apply_lighting.fx"), [ _post_process_output, _position_output, "normal", "metallic_roughness", "light_map", "irradiance_map", "prefiltered", "brdf_lut", "ssao_target" ], [ flipFlopPostProcess() ])
  }
  ssao(strength, blur_scale, blur_passes, render_target_scale) {
    if (strength <= 0 || strength > 5) {
      PostProcess.addRenderTarget("ssao_target", Texture.create(Vec2.new(1.0), [ 255 ], TextureFormat.A8))
      return
    }

    // Add the required render targets.
    PostProcess.addRenderTarget("ssao_target", render_target_scale, TextureFormat.A8)
    PostProcess.addRenderTarget("ssao_target_temp", render_target_scale, TextureFormat.A8)
    PostProcess.addRenderTarget("random_texture", Texture.load("resources/textures/lighting/noise.png"))

    // Add the main SSAO pass.
    PostProcess.addShaderPass("ssao", Shader.load("resources/shaders/ssao.fx|STRENGTH%(strength)"), [ _position_output, "normal", "random_texture", "depth_buffer" ], [ "ssao_target" ])

    // // Horizontal blur.
    var shader_blur_x = Shader.load("resources/shaders/blur_7x1.fx|HORIZONTAL")

    var input = "ssao_target"
    var output = "ssao_target_temp"

    var i = 0
    while (i < blur_passes.x && blur_scale.x > 0.0) {
      PostProcess.addShaderPass("ssao_blur_x_%(i)", shader_blur_x, [ input ], [ output ])
      i = i + 1
      {
        var temp = input
        input = output
        output = temp
      }
    }

    // Vertical blur.
    var shader_blur_y = Shader.load("resources/shaders/blur_7x1.fx|VERTICAL")

    i = 0
    while (i < blur_passes.y && blur_scale.y > 0.0) {
      PostProcess.addShaderPass("ssao_blur_y_%(i)", shader_blur_y, [ input ], [ output ])
      i = i + 1
      {
        var temp = input
        input = output
        output = temp
      }
    }
  }

  skyDome(enabled) {
    if (!enabled) return

    var shader = Shader.load("resources/shaders/skydome.fx")
    PostProcess.addShaderPass("skydome", shader, [ _post_process_output, _position_output, "environment_map" ], [ flipFlopPostProcess(), flipFlopPosition() ])
  }
  bloom(enabled, blur_scale, blur_passes, render_target_scale) {
    if (!enabled) return

    // Add the render target.
    PostProcess.addRenderTarget("bloom", render_target_scale, TextureFormat.R16G16B16A16)
    PostProcess.addRenderTarget("bloom_temp", render_target_scale, TextureFormat.R16G16B16A16)
    _bloom_output = "bloom"

    // Bloom extract.
    var shader_extract = Shader.load("resources/shaders/bloom_extract.fx")
    PostProcess.addShaderPass("bloom_extract", shader_extract, [ _post_process_output ], [ _bloom_output ])

    // Bloom blur horizontal.
    var shader_blur_x  = Shader.load("resources/shaders/blur_7x1.fx|HORIZONTAL")

    var i = 0
    while (i < blur_passes.x && blur_scale.x > 0.0) {
      PostProcess.addShaderPass("bloom_blur_x_%(i)", shader_blur_x, [ _bloom_output ], [ flipFlopBloom() ])
      i = i + 1
    }

    // Bloom blur vertical.
    var shader_blur_y  = Shader.load("resources/shaders/blur_7x1.fx|VERTICAL")

    i = 0
    while (i < blur_passes.y && blur_scale.y > 0.0) {
      PostProcess.addShaderPass("bloom_blur_y_%(i)", shader_blur_y, [ _bloom_output ], [ flipFlopBloom() ])
      i = i + 1
    }

    // Bloom apply.
    PostProcess.addShaderPass("bloom_apply", Shader.load("resources/shaders/bloom_apply.fx"), [ _post_process_output, _bloom_output ], [ flipFlopPostProcess() ])
  }
  toneMapping(enabled) {
    if (!enabled) return

    PostProcess.addShaderPass("tone_mapping", Shader.load("resources/shaders/tone_mapping.fx"), [ _post_process_output ], [ flipFlopPostProcess() ])
  }
  fxaa(enabled) {
    if (!enabled) return

    PostProcess.addShaderPass("fxaa", Shader.load("resources/shaders/fxaa.fx"), [ _post_process_output ], [ flipFlopPostProcess() ])
  }
  dof(enabled, blur_scale, blur_passes, render_target_scale) {
    if (!enabled) return

    // Add required render targets.
    PostProcess.addRenderTarget("dof", render_target_scale, TextureFormat.R16G16B16A16)
    PostProcess.addRenderTarget("dof_temp", render_target_scale, TextureFormat.R16G16B16A16)
    PostProcess.addRenderTarget("dof_pos", Texture.create(Vec2.new(1.0), [ 0, 0 ], TextureFormat.R16))
    PostProcess.addRenderTarget("dof_pos_temp", Texture.create(Vec2.new(1.0), [ 0, 0 ], TextureFormat.R16))
    _dof_output = "dof"
    _dof_pos_output = "dof_pos"

    // Blur horizontal.
    var shader_blur_x = Shader.load("resources/shaders/blur_7x1.fx|HORIZONTAL")

    var i = 0
    while (i < blur_passes.x && blur_scale.x > 0.0) {
      PostProcess.addShaderPass("dof_blur_x_%(i)", shader_blur_x, [ (i == 0 ? _post_process_output : _dof_output) ], [ flipFlopDof() ])
      i = i + 1
    }

    // Blur vertical.
    var shader_blur_y = Shader.load("resources/shaders/blur_7x1.fx|VERTICAL")

    i = 0
    while (i < blur_passes.y && blur_scale.y > 0.0) {
      PostProcess.addShaderPass("dof_blur_y_%(i)", shader_blur_y, [ _dof_output ], [ flipFlopDof() ])
      i = i + 1
    }

    // Position update.
    PostProcess.addShaderPass("dof_pos", Shader.load("resources/shaders/dof_pos.fx"), [ _dof_pos_output, _position_output ], [ flipFlopDofPos() ])

    // Apply.
    PostProcess.addShaderPass("dof_apply", Shader.load("resources/shaders/dof.fx"), [ _post_process_output, _dof_output, _dof_pos_output, _position_output ], [ flipFlopPostProcess() ])
  }
  water(enabled) {
    if (!enabled) return
    PostProcess.addShaderPass("water", Shader.load("resources/shaders/water.fx"), [ _post_process_output ], [ flipFlopPostProcess() ])
  }
  ssr(enabled) {
    if (!enabled) return
  PostProcess.addShaderPass("ssr", Shader.load("resources/shaders/ssr.fx"), [ _post_process_output, _position_output, "normal" ], [ flipFlopPostProcess() ])
  }
  gui(enabled) {
    GUI.enabled = enabled
  }
}
