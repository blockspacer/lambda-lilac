import "Core/Vec2" for Vec2
import "Core/Vec3" for Vec3
import "Core/Vec4" for Vec4

import "Core/Texture" for Texture, TextureFormat
import "Core/Shader" for Shader
import "Core/Mesh" for Mesh

import "Core/Graphics" for Graphics
import "Core/PostProcess" for PostProcess

class Settings {
    construct new() {}
    lighting_enabled { _lighting_enabled }
    ssao_enabled { _ssao_enabled }
    ssao_blur_scale { _ssao_blur_scale }
    ssao_blur_passes { _ssao_blur_passes }
    ssao_rt_scale { _ssao_rt_scale }
    skydome_enabled { _skydome_enabled }
    bloom_enabled { _bloom_enabled }
    bloom_blur_scale { _bloom_blur_scale }
    bloom_blur_passes { _bloom_blur_passes }
    bloom_rt_scale { _bloom_rt_scale }
    tone_mapping_enabled { _tone_mapping_enabled }
    fxaa_enabled { _fxaa_enabled }
    dof_enabled { _dof_enabled }
    dof_blur_scale { _dof_blur_scale }
    dof_blur_passes { _dof_blur_passes }
    dof_rt_scale { _dof_rt_scale }
    water_enabled { _water_enabled }
    gui_enabled { _gui_enabled }
    lighting_enabled=(v) { _lighting_enabled = v }
    ssao_enabled=(v) { _ssao_enabled = v }
    ssao_blur_scale=(v) { _ssao_blur_scale = v }
    ssao_blur_passes=(v) { _ssao_blur_passes = v }
    ssao_rt_scale=(v) { _ssao_rt_scale = v }
    skydome_enabled=(v) { _skydome_enabled = v }
    bloom_enabled=(v) { _bloom_enabled = v }
    bloom_blur_scale=(v) { _bloom_blur_scale = v }
    bloom_blur_passes=(v) { _bloom_blur_passes = v }
    bloom_rt_scale=(v) { _bloom_rt_scale = v }
    tone_mapping_enabled=(v) { _tone_mapping_enabled = v }
    fxaa_enabled=(v) { _fxaa_enabled = v }
    dof_enabled=(v) { _dof_enabled = v }
    dof_blur_scale=(v) { _dof_blur_scale = v }
    dof_blur_passes=(v) { _dof_blur_passes = v }
    dof_rt_scale=(v) { _dof_rt_scale = v }
    water_enabled=(v) { _water_enabled = v }
    gui_enabled=(v) { _gui_enabled = v }
}

class PostProcessor {
    construct new() {
        /*Utility::IniReader ini_reader("resources/settings.ini")

        Settings settings
        settings.lighting_enabled     = ini_reader.GetBool ("Lighting",    "Enabled")
        settings.ssao_enabled         = ini_reader.GetBool ("SSAO",        "Enabled")
        settings.ssao_blur_scale.x    = ini_reader.GetFloat("SSAO",        "BlurScaleX")
        settings.ssao_blur_scale.y    = ini_reader.GetFloat("SSAO",        "BlurScaleY")
        settings.ssao_blur_passes.x   = ini_reader.GetFloat("SSAO",        "BlurPassesX")
        settings.ssao_blur_passes.y   = ini_reader.GetFloat("SSAO",        "BlurPassesY")
        settings.ssao_rt_scale        = ini_reader.GetFloat("SSAO",        "TargetScale")
        settings.skydome_enabled      = ini_reader.GetBool ("Skydome",     "Enabled")
        settings.bloom_enabled        = ini_reader.GetBool ("Bloom",       "Enabled")
        settings.bloom_blur_scale.x   = ini_reader.GetFloat("Bloom",       "BlurScaleX")
        settings.bloom_blur_scale.y   = ini_reader.GetFloat("Bloom",       "BlurScaleY")
        settings.bloom_blur_passes.x  = ini_reader.GetFloat("Bloom",       "BlurPassesX")
        settings.bloom_blur_passes.y  = ini_reader.GetFloat("Bloom",       "BlurPassesY")
        settings.bloom_rt_scale       = ini_reader.GetFloat("Bloom",       "TargetScale")
        settings.tone_mapping_enabled = ini_reader.GetBool ("ToneMapping", "Enabled")
        settings.fxaa_enabled         = ini_reader.GetBool ("FXAA",        "Enabled")
        settings.dof_enabled          = ini_reader.GetBool ("DOF",         "Enabled")
        settings.dof_blur_scale.x     = ini_reader.GetFloat("DOF",         "BlurScaleX")
        settings.dof_blur_scale.y     = ini_reader.GetFloat("DOF",         "BlurScaleY")
        settings.dof_blur_passes.x    = ini_reader.GetFloat("DOF",         "BlurPassesX")
        settings.dof_blur_passes.y    = ini_reader.GetFloat("DOF",         "BlurPassesY")
        settings.dof_rt_scale         = ini_reader.GetFloat("DOF",         "TargetScale")
        settings.water_enabled        = ini_reader.GetBool ("Water",       "Enabled")
        settings.gui_enabled          = ini_reader.GetBool ("GUI",         "Enabled")
        */

        var settings = Settings.new()
        settings.lighting_enabled     = true
        settings.ssao_enabled         = false
        settings.ssao_blur_scale      = Vec2.new(1.0)
        settings.ssao_blur_passes     = Vec2.new(0.0)
        settings.ssao_rt_scale        = 0.5
        settings.skydome_enabled      = true
        settings.bloom_enabled        = false
        settings.bloom_blur_scale     = Vec2.new(1.0)
        settings.bloom_blur_passes    = Vec2.new(3.0)
        settings.bloom_rt_scale       = 0.5
        settings.tone_mapping_enabled = true
        settings.fxaa_enabled         = true
        settings.dof_enabled          = false
        settings.dof_blur_scale       = Vec2.new(1.0)
        settings.dof_blur_passes      = Vec2.new(3.0)
        settings.dof_rt_scale         = 0.5
        settings.water_enabled        = false
        settings.gui_enabled          = true

        PostProcess.addRenderTarget("albedo",              1.0, TextureFormat.R8G8B8A8)
        PostProcess.addRenderTarget("position",            1.0, TextureFormat.R32G32B32A32)
        PostProcess.addRenderTarget("normal",              1.0, TextureFormat.R8G8B8A8)
        PostProcess.addRenderTarget("metallic_roughness",  1.0, TextureFormat.R8G8B8A8)
        PostProcess.addRenderTarget("depth_buffer",        1.0, TextureFormat.R24G8)
        PostProcess.addRenderTarget("post_process_buffer", 1.0, TextureFormat.R32G32B32A32)

        PostProcess.addRenderTarget("brdf_lut", Texture.load("resources/textures/ibl_brdf_lut.png"))
        PostProcess.addRenderTarget("environment_map", Texture.load("resources/textures/bell_park_dawn.hdr"))
        PostProcess.irradianceConvolution("environment_map", "irradiance_map")
        PostProcess.hammerhead("environment_map", "prefiltered")


        // Create all passes.
        copyAlbedoToPostProcessBuffer()
        shadowMapping(settings.lighting_enabled)
        applyLighting(settings.lighting_enabled)
        ssao(settings.ssao_enabled, settings.ssao_blur_scale, settings.ssao_blur_passes, settings.ssao_rt_scale)
        skyDome(settings.skydome_enabled)
        bloom(settings.bloom_enabled, settings.bloom_blur_scale, settings.bloom_blur_passes, settings.bloom_rt_scale)
        toneMapping(settings.tone_mapping_enabled)
        fxaa(settings.fxaa_enabled)
        dof(settings.dof_enabled, settings.dof_blur_scale, settings.dof_blur_passes, settings.dof_rt_scale)
        water(settings.water_enabled)
        gui(settings.gui_enabled)

        // Final pass.
        PostProcess.setFinalRenderTarget("post_process_buffer")
    }

    copyAlbedoToPostProcessBuffer() {
        PostProcess.addShaderPass("apply_lighting", Shader.load("resources/shaders/copy.fx"), [ "albedo" ], [ "post_process_buffer" ])
    }

    shadowMapping(enabled) {
        if(!enabled) return

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
        if(!enabled) return    
        PostProcess.addShaderPass("apply_lighting", Shader.load("resources/shaders/apply_lighting.fx"), [ "post_process_buffer", "position", "normal", "metallic_roughness", "light_map", "irradiance_map", "prefiltered", "brdf_lut" ], [ "post_process_buffer" ])
    }
    ssao(enabled, blur_scale, blur_passes, render_target_scale) {
        if(!enabled) return

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
        if(!enabled) return

        var shader = Shader.load("resources/shaders/skydome.fx")
        PostProcess.addShaderPass("skydome", shader, [ "post_process_buffer", "position", "environment_map" ], [ "post_process_buffer", "position" ])
    }
    bloom(enabled, blur_scale, blur_passes, render_target_scale) {
        if(!enabled) return

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
        if(!enabled) return

        PostProcess.addShaderPass("tone_mapping", Shader.load("resources/shaders/tone_mapping.fx"), [ "post_process_buffer" ], [ "post_process_buffer" ])
    }
    fxaa(enabled) {
        if(!enabled) return

        PostProcess.addShaderPass("fxaa", Shader.load("resources/shaders/fxaa.fx"), [ "post_process_buffer" ], [ "post_process_buffer" ])
    }
    dof(enabled, blur_scale, blur_passes, render_target_scale) {
        if(!enabled) return

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
        if(!enabled) return
        PostProcess.addShaderPass("water", Shader.load("resources/shaders/water.fx"), [ "post_process_buffer" ], [ "post_process_buffer" ])
    }
    gui(enabled) {
        if(!enabled) return
        //PostProcess.addShaderPass("apply_gui", Shader.load("resources/shaders/apply_gui.fx"), [ "post_process_buffer", "gui" ], [ "post_process_buffer" ])
    }
}
