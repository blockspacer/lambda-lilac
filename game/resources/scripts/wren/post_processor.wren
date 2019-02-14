import "Core/Vec2" for Vec2
import "Core/Vec3" for Vec3
import "Core/Vec4" for Vec4

import "Core/Texture" for Texture, TextureFormat
import "Core/Shader" for Shader
import "Core/Mesh" for Mesh

import "Core/Graphics" for Graphics
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
    PostProcess.addRenderTarget("depth_buffer",        1.0, TextureFormat.R24G8)
    PostProcess.addRenderTarget("post_process_buffer", 1.0, TextureFormat.R32G32B32A32)

    // Set the final pass.
    PostProcess.setFinalRenderTarget("post_process_buffer")

    // Add and generate the environment maps.
    PostProcess.addRenderTarget("brdf_lut", Texture.load("resources/textures/ibl_brdf_lut.png"))
    PostProcess.addRenderTarget("environment_map", Texture.load("resources/textures/bell_park_dawn.hdr"))
    PostProcess.irradianceConvolution("environment_map", "irradiance_map")
    PostProcess.hammerhead("environment_map", "prefiltered")

    // Create all passes.
    copyAlbedoToPostProcessBuffer()
    shadowMapping(
      ini_reader["Lighting", "Enabled"]
    )
    applyLighting(
      ini_reader["Lighting", "Enabled"]
    )
    ssao(
      ini_reader["SSAO", "Enabled"], 
      Vec2.new(ini_reader["SSAO", "BlurScaleX"], ini_reader["SSAO", "BlurScaleY"]), 
      Vec2.new(ini_reader["SSAO", "BlurPassesX"], ini_reader["SSAO", "BlurPassesY"]), 
      ini_reader["SSAO", "TargetScale"]
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
    PostProcess.addShaderPass("apply_lighting", Shader.load("resources/shaders/copy.fx"), [ "albedo" ], [ "post_process_buffer" ])
  }

  shadowMapping(enabled) {
    if (!enabled) return

    // Get all shared shaders.
    var generate = Shader.load("resources/shaders/vsm_generate.fx")
    var blur_x = Shader.load("resources/shaders/vsm_blur_7x1.fx")
    var blur_y = Shader.load("resources/shaders/vsm_blur_7x1.fx")
    blur_x.setVariableFloat2("blur_scale", Vec2.new(1.0, 0.0))
    blur_y.setVariableFloat2("blur_scale", Vec2.new(0.0, 1.0))
    var modify = [ blur_x, blur_y ]

    // Set all lighting shaders.
    Graphics.setDirectionalShaders(generate, modify, Shader.load("resources/shaders/vsm_publish_directional.fx"))
    Graphics.setPointLightShaders( generate, modify, Shader.load("resources/shaders/vsm_publish_point.fx"))
    Graphics.setSpotLightShaders(  generate, modify, Shader.load("resources/shaders/vsm_publish_spot.fx"))
    Graphics.setCascadeShaders(    generate, modify, Shader.load("resources/shaders/vsm_publish_cascade.fx"))

    // Set RSM shaders.
    // I'm actually using Reflective Variance Shadow Maps.
    var rsm_generate = Shader.load("resources/shaders/rvsm_generate.fx")
    Graphics.setDirectionalShadersRSM(rsm_generate, modify, Shader.load("resources/shaders/rvsm_publish_directional.fx"))
    Graphics.setSpotLightShadersRSM(rsm_generate, modify, Shader.load("resources/shaders/rvsm_publish_spot.fx"))
  }
  applyLighting(enabled) {
    if (!enabled) return

    PostProcess.addShaderPass("apply_lighting", Shader.load("resources/shaders/apply_lighting.fx"), [ "post_process_buffer", "position", "normal", "metallic_roughness", "light_map", "irradiance_map", "prefiltered", "brdf_lut" ], [ "post_process_buffer" ])
  }
  ssao(enabled, blur_scale, blur_passes, render_target_scale) {
    if (!enabled) return

    // Add the required render targets.
    PostProcess.addRenderTarget("ssao_target", render_target_scale, TextureFormat.R32G32B32A32)
    PostProcess.addRenderTarget("random_texture", Texture.load("resources/textures/noise.png"))

    // Add the main SSAO pass.
    PostProcess.addShaderPass("ssao", Shader.load("resources/shaders/ssao.fx"), [ "position", "normal", "random_texture", "depth_buffer" ], [ "ssao_target" ])

    // Horizontal blur.
    var shader_blur_x = Shader.load("resources/shaders/blur_9x1.fx")
    shader_blur_x.setVariableFloat2("blur_scale", Vec2.new(blur_scale.x, 0.0))

    var i = 0
    while (i < blur_passes.x && blur_scale.x > 0.0) {
      PostProcess.addShaderPass("ssao_blur_x_%(i)", shader_blur_x, [ "ssao_target" ], [ "ssao_target" ])
      i = i + 1
    }

    // Vertical blur.
    var shader_blur_y = Shader.load("resources/shaders/blur_9x1.fx")
    shader_blur_y.setVariableFloat2("blur_scale", Vec2.new(0.0, blur_scale.y))

    i = 0
    while (i < blur_passes.y && blur_scale.y > 0.0) {
      PostProcess.addShaderPass("ssao_blur_y_%(i)", shader_blur_y, [ "ssao_target" ], [ "ssao_target" ])
      i = i + 1
    }

    // Apply SSAO.
    var shader_apply = Shader.load("resources/shaders/ssao_apply.fx")
    PostProcess.addShaderPass("ssao_apply", shader_apply, [ "post_process_buffer", "ssao_target" ], [ "post_process_buffer" ])
  }

  skyDome(enabled) {
    if (!enabled) return

    var shader = Shader.load("resources/shaders/skydome.fx")
    PostProcess.addShaderPass("skydome", shader, [ "post_process_buffer", "position", "environment_map" ], [ "post_process_buffer", "position" ])
  }
  bloom(enabled, blur_scale, blur_passes, render_target_scale) {
    if (!enabled) return

    // Add the render target.
    PostProcess.addRenderTarget("bloom_target", render_target_scale, TextureFormat.R32G32B32A32)

    // Bloom extract.
    var shader_extract = Shader.load("resources/shaders/bloom_extract.fx")
    PostProcess.addShaderPass("bloom_extract", shader_extract, [ "post_process_buffer" ], [ "bloom_target" ])

    // Bloom blur horizontal.
    var shader_blur_x  = Shader.load("resources/shaders/blur_9x1.fx")
    shader_blur_x.setVariableFloat2("blur_scale", Vec2.new(blur_scale.x, 0.0))

    var i = 0
    while (i < blur_passes.x && blur_scale.x > 0.0) {
      PostProcess.addShaderPass("bloom_blur_x_%(i)", shader_blur_x, [ "bloom_target" ], [ "bloom_target" ])
      i = i + 1
    }

    // Bloom blur vertical.
    var shader_blur_y  = Shader.load("resources/shaders/blur_9x1.fx")
    shader_blur_y.setVariableFloat2("blur_scale", Vec2.new(0.0, blur_scale.y))

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
    PostProcess.addRenderTarget("dof_target", render_target_scale, TextureFormat.R32G32B32A32)
    PostProcess.addRenderTarget("dof_pos", Texture.create(Vec2.new(1.0), [ 16.0 ], TextureFormat.R32G32B32A32))

    // Blur horizontal.
    var shader_blur_x = Shader.load("resources/shaders/blur_7x1.fx")
    shader_blur_x.setVariableFloat2("blur_scale", Vec2.new(blur_scale.x, 0.0))

    var i = 0
    while (i < blur_passes.x && blur_scale.x > 0.0) {
      PostProcess.addShaderPass("dof_blur_x_%(i)", shader_blur_x, [ (i == 0 ? "post_process_buffer" : "dof_target") ], [ "dof_target" ])
      i = i + 1
    }

    // Blur vertical.
    var shader_blur_y = Shader.load("resources/shaders/blur_7x1.fx")
    shader_blur_y.setVariableFloat2("blur_scale", Vec2.new(blur_scale.y, 0.0))

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
    if (!enabled) return
    PostProcess.addShaderPass("apply_gui", Shader.load("resources/shaders/apply_gui.fx"), [ "post_process_buffer", "gui" ], [ "post_process_buffer" ])
  }
}
