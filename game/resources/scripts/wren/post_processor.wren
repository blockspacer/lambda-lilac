import "Core/Vec2" for Vec2
import "Core/Vec3" for Vec3
import "Core/Vec4" for Vec4

import "Core/Texture" for Texture, TextureFormat
import "Core/Shader" for Shader
import "Core/Mesh" for Mesh

import "Core/Graphics" for Graphics
import "Core/GUI" for GUI
import "Core/PostProcess" for PostProcess

import "resources/scripts/wren/ini" for Ini

class PostProcessor {
  construct new() {
    // Load the .INI file.
    var ini_reader = Ini.new("resources/settings.ini")
    
    // Create all required render targets.
    PostProcess.addRenderTarget("albedo",              1.0, TextureFormat.R8G8B8A8)
    PostProcess.addRenderTarget("position",            1.0, TextureFormat.R32G32B32A32)
    PostProcess.addRenderTarget("normal",              1.0, TextureFormat.R8G8B8A8)
    PostProcess.addRenderTarget("metallic_roughness",  1.0, TextureFormat.R8G8B8A8)
    PostProcess.addRenderTarget("depth_buffer",        1.0, TextureFormat.D32)
    PostProcess.addRenderTarget("post_process_buffer", 1.0, TextureFormat.R16G16B16A16)

    // Set the final pass.
    PostProcess.setFinalRenderTarget("post_process_buffer")

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
  }

  copyAlbedoToPostProcessBuffer() {
    PostProcess.addShaderPass("copy_albedo_to_post_process_buffer", Shader.load("resources/shaders/copy.fx"), [ "albedo" ], [ "post_process_buffer" ])
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
    PostProcess.addShaderPass("apply_lighting", Shader.load("resources/shaders/apply_lighting.fx"), [ "post_process_buffer", "position", "normal", "metallic_roughness", "light_map", "irradiance_map", "prefiltered", "brdf_lut", "ssao_target" ], [ "post_process_buffer" ])
  }
  ssao(strength, blur_scale, blur_passes, render_target_scale) {
    if (strength == 0 || strength > 5) {
      PostProcess.addRenderTarget("ssao_target", Texture.create(Vec2.new(1.0), [ 255 ], TextureFormat.A8))
      return
    }

    // Add the required render targets.
    PostProcess.addRenderTarget("ssao_target", render_target_scale, TextureFormat.A8)
    PostProcess.addRenderTarget("random_texture", Texture.load("resources/textures/lighting/noise.png"))

    // Add the main SSAO pass.
    PostProcess.addShaderPass("ssao", Shader.load("resources/shaders/ssao.fx|STRENGTH%(strength)"), [ "position", "normal", "random_texture", "depth_buffer" ], [ "ssao_target" ])

    // // Horizontal blur.
    var shader_blur_x = Shader.load("resources/shaders/blur_7x1.fx|HORIZONTAL")

    var i = 0
    while (i < blur_passes.x && blur_scale.x > 0.0) {
      PostProcess.addShaderPass("ssao_blur_x_%(i)", shader_blur_x, [ "ssao_target" ], [ "ssao_target" ])
      i = i + 1
    }

    // Vertical blur.
    var shader_blur_y = Shader.load("resources/shaders/blur_7x1.fx|VERTICAL")

    i = 0
    while (i < blur_passes.y && blur_scale.y > 0.0) {
      PostProcess.addShaderPass("ssao_blur_y_%(i)", shader_blur_y, [ "ssao_target" ], [ "ssao_target" ])
      i = i + 1
    }
  }

  skyDome(enabled) {
    if (!enabled) return

    var shader = Shader.load("resources/shaders/skydome.fx")
    PostProcess.addShaderPass("skydome", shader, [ "post_process_buffer", "position", "environment_map" ], [ "post_process_buffer", "position" ])
  }
  bloom(enabled, blur_scale, blur_passes, render_target_scale) {
    if (!enabled) return

    // Add the render target.
    PostProcess.addRenderTarget("bloom_target", render_target_scale, TextureFormat.R16G16B16A16)

    // Bloom extract.
    var shader_extract = Shader.load("resources/shaders/bloom_extract.fx")
    PostProcess.addShaderPass("bloom_extract", shader_extract, [ "post_process_buffer" ], [ "bloom_target" ])

    // Bloom blur horizontal.
    var shader_blur_x  = Shader.load("resources/shaders/blur_7x1.fx|HORIZONTAL")

    var i = 0
    while (i < blur_passes.x && blur_scale.x > 0.0) {
      PostProcess.addShaderPass("bloom_blur_x_%(i)", shader_blur_x, [ "bloom_target" ], [ "bloom_target" ])
      i = i + 1
    }

    // Bloom blur vertical.
    var shader_blur_y  = Shader.load("resources/shaders/blur_7x1.fx|VERTICAL")

    i = 0
    while (i < blur_passes.y && blur_scale.y > 0.0) {
      PostProcess.addShaderPass("bloom_blur_y_%(i)", shader_blur_y, [ "bloom_target" ], [ "bloom_target" ])
      i = i + 1
    }

    // Bloom apply.
    PostProcess.addShaderPass("bloom_apply", Shader.load("resources/shaders/bloom_apply.fx"), [ "post_process_buffer", "bloom_target" ], [ "post_process_buffer" ])
  }
  toneMapping(enabled) {
    if (!enabled) return

    PostProcess.addShaderPass("tone_mapping", Shader.load("resources/shaders/tone_mapping.fx"), [ "post_process_buffer" ], [ "post_process_buffer" ])
  }
  fxaa(enabled) {
    if (!enabled) return

    PostProcess.addShaderPass("fxaa", Shader.load("resources/shaders/fxaa.fx"), [ "post_process_buffer" ], [ "post_process_buffer" ])
  }
  dof(enabled, blur_scale, blur_passes, render_target_scale) {
    if (!enabled) return

    // Add required render targets.
    PostProcess.addRenderTarget("dof_target", render_target_scale, TextureFormat.R16G16B16A16)
    PostProcess.addRenderTarget("dof_pos", Texture.create(Vec2.new(1.0), [ 0, 0 ], TextureFormat.R16))

    // Blur horizontal.
    var shader_blur_x = Shader.load("resources/shaders/blur_7x1.fx|HORIZONTAL")

    var i = 0
    while (i < blur_passes.x && blur_scale.x > 0.0) {
      PostProcess.addShaderPass("dof_blur_x_%(i)", shader_blur_x, [ (i == 0 ? "post_process_buffer" : "dof_target") ], [ "dof_target" ])
      i = i + 1
    }

    // Blur vertical.
    var shader_blur_y = Shader.load("resources/shaders/blur_7x1.fx|VERTICAL")

    i = 0
    while (i < blur_passes.y && blur_scale.y > 0.0) {
      PostProcess.addShaderPass("dof_blur_y_%(i)", shader_blur_y, [ "dof_target" ], [ "dof_target" ])
      i = i + 1
    }

    // Position update.
    PostProcess.addShaderPass("dof_pos", Shader.load("resources/shaders/dof_pos.fx"), [ "dof_pos", "position" ], [ "dof_pos" ])

    // Apply.
    PostProcess.addShaderPass("dof_apply", Shader.load("resources/shaders/dof.fx"), [ "post_process_buffer", "dof_target", "dof_pos", "position" ], [ "post_process_buffer" ])
  }
  water(enabled) {
    if (!enabled) return
    PostProcess.addShaderPass("water", Shader.load("resources/shaders/water.fx"), [ "post_process_buffer" ], [ "post_process_buffer" ])
  }
  ssr(enabled) {
    if (!enabled) return
    PostProcess.addShaderPass("ssr", Shader.load("resources/shaders/ssr.fx"), [ "post_process_buffer", "position", "normal" ], [ "post_process_buffer" ])
  }
  gui(enabled) {
    GUI.enabled = enabled
  }
}
