/** @file post_process.as */
#include "asset_manager.as"
#include "assets.as"

/**
* @brief Adds a render target which will size will be dynamic, based upon the window size
* @param name (const String) The name of the new render target
* @param render_scale (const float) The scale of the new render target relative to the window size
* @param format (const Asset::TextureFormat) The texture format of the new render target
* @author Hilze Vonck
**/
void AddRenderTarget(const String&in name, const float&in render_scale, const Asset::TextureFormat&in format)
{
  Asset::Texture texture = asset_manager.CreateTexture(Vec2(1.0f), format);
  Violet_Utilities_RenderTarget::Register(name, render_scale, texture.GetId(), false);
}
/**
* @brief Adds a texture as a render target
* @param name (const String) The name of the new render target
* @param texture (const Asset::Texture&) The texture which will be accessible as a render target
* @author Hilze Vonck
**/
void AddRenderTarget(const String&in name, const Asset::Texture&in texture)
{
  Violet_Utilities_RenderTarget::Register(name, 0.0f, texture.GetId(), true);
}
void SetRenderTargetFlag(const String&in name, const RenderTargetFlags&in flag, const bool&in value)
{
  Violet_Utilities_RenderTarget::SetFlag(name, uint8(flag), value);
}
void Hammerhead(const String&in input, const String&in output)
{
  Violet_Utilities_RenderTarget::Hammerhead(input, output);
}
void IrradianceConvolution(const String&in input, const String&in output)
{
  Violet_Utilities_RenderTarget::IrradianceConvolution(input, output);
}

/**
* @brief Sets the render target that will be copied over to the screen all the way at the end of the rendering pipeline
* @param name (const String) The name of the render target that should be copied over to the screen
* @author Hilze Vonck
**/
void SetFinalRenderTarget(const String&in name)
{
  Violet_Utilities_RenderTarget::SetFinalRenderTarget(name);
}
/**
* @brief Adds a shader pass to the post processing pipeline
* @param name (const String) The name of the shader pass
* @param shader (const Asset::Shader&) The shader that this shader pass should use
* @param input (const Array<String>&) The textures / render targets that this shader pass requires as input
* @param output (const Array<String>&) The render targets that this shader pass will render to
* @author Hilze Vonck
**/
void AddShaderPass(const String&in name, const Asset::Shader&in shader, const Array<String>&in input, const Array<String>&in output)
{
  Violet_Utilities_ShaderPass::Register(name, shader.GetId(), input, output);
}
void SetShaderPassEnabled(const String&in name, const bool&in enabled)
{
  Violet_Utilities_ShaderPass::SetEnabled(name, enabled);
}
void SetShaderVariable(const String&in name, const float&in value)
{
  Violet_Utilities_ShaderPass::SetShaderVariable(name, value);
}
void SetShaderVariable(const String&in name, const Vec2&in value)
{
  Violet_Utilities_ShaderPass::SetShaderVariable(name, value);
}
void SetShaderVariable(const String&in name, const Vec3&in value)
{
  Violet_Utilities_ShaderPass::SetShaderVariable(name, value);
}
