/**
* @class Graphics
* @brief Helper class which makes it easy for you to set and get graphics values
* @brief A static instance extist: graphics
* @author Hilze Vonck
**/
class Graphics
{
    /**
    * @brief Sets virtual synchronization
    * @param vsync (const bool) The new vsync value
    * @public
    **/
    void SetVSync(const bool&in vsync)
    {
        Violet_Graphics_Renderer::SetVSync(vsync);
    }
    /**
    * @brief Sets the renderers render scale 1.0f is equal to 100%. 0.5f to 50%, 2.0f to 200%, etc..
    * @param render_scale (const float) The new render scale
    * @public
    **/
    void SetRenderScale(const float&in render_scale)
    {
        Violet_Graphics_Renderer::SetRenderScale(render_scale);
    }
    /**
    * @brief Sets the shaders that the light system uses to render lights
    * @brief The generate shader is used for the generation of the shadow maps<br>
    * @brief The modify shaders stands for the modifications that need to happen to some shadow maps. E.G., vsm blur<br>
    * @brief The publish shader is used for the rendering of the lights, both with and without shadow maps
    * @param generate (const Asset::Shader&) The new generate shader
    * @param modify (const Array<Asset::Shader>&) The new modify shaders
    * @param publish (const Asset::Shader&) The new publish shader
    * @public
    **/
    void SetDirectionalLightShaders(const Asset::Shader&in generate, const Array<Asset::Shader>&in modify, const Asset::Shader&in publish)
    {
        Array<uint64> ids;
        for(uint64 i = 0; i < modify.Size(); ++i)
        {
            ids.PushBack(modify[i].GetId());
        }

        Violet_Graphics_Globals::SetShadersDirectional(generate.GetId(), ids, publish.GetId());
    }
    /**
    * @brief Sets the shaders that the light system uses to render lights
    * @brief The generate shader is used for the generation of the shadow maps<br>
    * @brief The modify shaders stands for the modifications that need to happen to some shadow maps. E.G., vsm blur<br>
    * @brief The publish shader is used for the rendering of the lights, both with and without shadow maps
    * @param generate (const Asset::Shader&) The new generate shader
    * @param modify (const Array<Asset::Shader>&) The new modify shaders
    * @param publish (const Asset::Shader&) The new publish shader
    * @public
    **/
    void SetSpotLightShaders(const Asset::Shader&in generate, const Array<Asset::Shader>&in modify, const Asset::Shader&in publish)
    {
        Array<uint64> ids;
        for(uint64 i = 0; i < modify.Size(); ++i)
        {
            ids.PushBack(modify[i].GetId());
        }

        Violet_Graphics_Globals::SetShadersSpot(generate.GetId(), ids, publish.GetId());
    }
    /**
    * @brief Sets the shaders that the light system uses to render lights
    * @brief The generate shader is used for the generation of the shadow maps<br>
    * @brief The modify shaders stands for the modifications that need to happen to some shadow maps. E.G., vsm blur<br>
    * @brief The publish shader is used for the rendering of the lights, both with and without shadow maps
    * @param generate (const Asset::Shader&) The new generate shader
    * @param modify (const Array<Asset::Shader>&) The new modify shaders
    * @param publish (const Asset::Shader&) The new publish shader
    * @public
    **/
    void SetPointLightShaders(const Asset::Shader&in generate, const Array<Asset::Shader>&in modify, const Asset::Shader&in publish)
    {
        Array<uint64> ids;
        for(uint64 i = 0; i < modify.Size(); ++i)
        {
            ids.PushBack(modify[i].GetId());
        }

        Violet_Graphics_Globals::SetShadersPoint(generate.GetId(), ids, publish.GetId());
    }
    /**
    * @brief Sets the shaders that the light system uses to render lights
    * @brief The generate shader is used for the generation of the shadow maps<br>
    * @brief The modify shaders stands for the modifications that need to happen to some shadow maps. E.G., vsm blur<br>
    * @brief The publish shader is used for the rendering of the lights, both with and without shadow maps
    * @param generate (const Asset::Shader&) The new generate shader
    * @param modify (const Array<Asset::Shader>&) The new modify shaders
    * @param publish (const Asset::Shader&) The new publish shader
    * @public
    **/
    void SetCascadeLightShaders(const Asset::Shader&in generate, const Array<Asset::Shader>&in modify, const Asset::Shader&in publish)
    {
        Array<uint64> ids;
        for(uint64 i = 0; i < modify.Size(); ++i)
        {
            ids.PushBack(modify[i].GetId());
        }

        Violet_Graphics_Globals::SetShadersCascade(generate.GetId(), ids, publish.GetId());
    }
    /**
    * @brief Sets the shaders that the light system uses to render lights
    * @brief The generate shader is used for the generation of the shadow maps<br>
    * @brief The modify shaders stands for the modifications that need to happen to some shadow maps. E.G., vsm blur<br>
    * @brief The publish shader is used for the rendering of the lights, both with and without shadow maps
    * @param generate (const Asset::Shader&) The new generate shader
    * @param modify (const Array<Asset::Shader>&) The new modify shaders
    * @param publish (const Asset::Shader&) The new publish shader
    * @public
    **/
    void SetDirectionalLightRSMShaders(const Asset::Shader&in generate, const Array<Asset::Shader>&in modify, const Asset::Shader&in publish)
    {
        Array<uint64> ids;
        for(uint64 i = 0; i < modify.Size(); ++i)
        {
            ids.PushBack(modify[i].GetId());
        }

        Violet_Graphics_Globals::SetShadersDirectionalRSM(generate.GetId(), ids, publish.GetId());
    }
    /**
    * @brief Sets the shaders that the light system uses to render lights
    * @brief The generate shader is used for the generation of the shadow maps<br>
    * @brief The modify shaders stands for the modifications that need to happen to some shadow maps. E.G., vsm blur<br>
    * @brief The publish shader is used for the rendering of the lights, both with and without shadow maps
    * @param generate (const Asset::Shader&) The new generate shader
    * @param modify (const Array<Asset::Shader>&) The new modify shaders
    * @param publish (const Asset::Shader&) The new publish shader
    * @public
    **/
    void SetSpotLightRSMShaders(const Asset::Shader&in generate, const Array<Asset::Shader>&in modify, const Asset::Shader&in publish)
    {
        Array<uint64> ids;
        for(uint64 i = 0; i < modify.Size(); ++i)
        {
            ids.PushBack(modify[i].GetId());
        }

        Violet_Graphics_Globals::SetShadersSpotRSM(generate.GetId(), ids, publish.GetId());
    }
}

Graphics graphics; //!< The static graphics instance
