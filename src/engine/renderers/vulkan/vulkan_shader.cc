#include "vulkan_shader.h"
#include <utils/file_system.h>
#include "vulkan_renderer.h"

namespace lambda
{
  namespace linux
  {
    ///////////////////////////////////////////////////////////////////////////
	  VulkanShader::VulkanShader(asset::VioletShaderHandle shader, VulkanRenderer* renderer)
		  : renderer_(renderer)
		  , vs_(VK_NULL_HANDLE)
		  , ps_(VK_NULL_HANDLE)
		  , gs_(VK_NULL_HANDLE)
		  , num_render_targets_(0u)
	  {
		  auto data = asset::ShaderManager::getInstance()->getData(shader);
		  Vector<VezPipelineShaderStageCreateInfo> shader_stages;

		  VkResult result;

		  VezShaderModuleCreateInfo shader_module_create_info{};
		  shader_module_create_info.pEntryPoint = "main";

		  VezPipelineShaderStageCreateInfo shader_stage{};
		  shader_stage.pEntryPoint = "main";

		  if (!data[(int)ShaderStages::kVertex][VIOLET_SPIRV].empty())
		  {
			  shader_module_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
			  shader_module_create_info.codeSize = static_cast<uint32_t>(data[(int)ShaderStages::kVertex][VIOLET_SPIRV].size());
			  shader_module_create_info.pCode = (uint32_t*)data[(int)ShaderStages::kVertex][VIOLET_SPIRV].data();

			  result = vezCreateShaderModule(renderer_->getDevice(), &shader_module_create_info, &vs_);
			  LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not create shader module | %s", vkErrorCode(result));

			  shader_stage.module = vs_;
			  shader_stages.push_back(shader_stage);
		  }

		  if (!data[(int)ShaderStages::kPixel][VIOLET_SPIRV].empty())
		  {
			  shader_module_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			  shader_module_create_info.codeSize = static_cast<uint32_t>(data[(int)ShaderStages::kPixel][VIOLET_SPIRV].size());
			  shader_module_create_info.pCode = reinterpret_cast<const uint32_t*>(data[(int)ShaderStages::kPixel][VIOLET_SPIRV].data());

			  result = vezCreateShaderModule(renderer_->getDevice(), &shader_module_create_info, &ps_);
			  LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not create shader module | %s", vkErrorCode(result));

			  shader_stage.module = ps_;
			  shader_stages.push_back(shader_stage);
		  }

		  if (!data[(int)ShaderStages::kGeometry][VIOLET_SPIRV].empty())
		  {
			  shader_module_create_info.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
			  shader_module_create_info.codeSize = static_cast<uint32_t>(data[(int)ShaderStages::kGeometry][VIOLET_SPIRV].size());
			  shader_module_create_info.pCode = reinterpret_cast<const uint32_t*>(data[(int)ShaderStages::kGeometry][VIOLET_SPIRV].data());

			  result = vezCreateShaderModule(renderer_->getDevice(), &shader_module_create_info, &gs_);
			  LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not create shader module | %s", vkErrorCode(result));

			  shader_stage.module = gs_;
			  shader_stages.push_back(shader_stage);
		  }

		   
		  VezGraphicsPipelineCreateInfo pipeline_create_info{};
		  pipeline_create_info.stageCount = static_cast<uint32_t>(shader_stages.size());
		  pipeline_create_info.pStages = shader_stages.data();
		  result = vezCreateGraphicsPipeline(renderer_->getDevice(), &pipeline_create_info, &pipeline_);
		  LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not create pipeline | %s", vkErrorCode(result));

		  reflect();
	  }
    
    ///////////////////////////////////////////////////////////////////////////
    VulkanShader::~VulkanShader()
    {
		vezDestroyShaderModule(renderer_->getDevice(), vs_);
		vezDestroyShaderModule(renderer_->getDevice(), ps_);
		vezDestroyShaderModule(renderer_->getDevice(), gs_);
		vezDestroyPipeline(renderer_->getDevice(), pipeline_);

      ps_buffers_.resize(0u);
      vs_buffers_.resize(0u);
      gs_buffers_.resize(0u);

      for (auto& buffer : ps_Vulkan_buffers_) 
        renderer_->freeRenderBuffer((platform::IRenderBuffer*&)buffer.buffer);
      for (auto& buffer : vs_Vulkan_buffers_) 
        renderer_->freeRenderBuffer((platform::IRenderBuffer*&)buffer.buffer);
      for (auto& buffer : gs_Vulkan_buffers_) 
        renderer_->freeRenderBuffer((platform::IRenderBuffer*&)buffer.buffer);
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanShader::bind()
    {
		vezCmdBindPipeline(pipeline_);
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanShader::unbind()
    {
      for (auto& buffer : vs_Vulkan_buffers_)
        buffer.bound = false;
      for (auto& buffer : ps_Vulkan_buffers_)
        buffer.bound = false;
      for (auto& buffer : gs_Vulkan_buffers_)
        buffer.bound = false;
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanShader::bindBuffers()
    {
      /*for (size_t i = 0u; i < vs_buffers_.size(); ++i)
      {
        auto& buffer = vs_buffers_.at(i);
        auto& Vulkan_buffer = vs_Vulkan_buffers_.at(i);

        if (buffer.getChanged() == true)
        {
          buffer.setChanged(false);
          context_->getVulkanContext()->UpdateSubresource(
            Vulkan_buffer.buffer->getBuffer(), 
            0u, 
            NULL, 
            buffer.getData(), 
            0u, 
            0u
          );
          Vulkan_buffer.bound = false;
        }
        if (!Vulkan_buffer.bound)
        {
          Vulkan_buffer.bound = true;
          IVulkanBuffer* buffer =
            (IVulkanBuffer*)Vulkan_buffer.buffer->getBuffer();
          
          context_->getVulkanContext()->VSSetConstantBuffers(
            (UINT)Vulkan_buffer.slot, 
            1u, 
            &buffer
          );
        }
      }
      for (size_t i = 0u; i < ps_buffers_.size(); ++i)
      {
        auto& buffer = ps_buffers_.at(i);
        auto& Vulkan_buffer = ps_Vulkan_buffers_.at(i);

        if (buffer.getChanged() == true)
        {
          buffer.setChanged(false);
          
          context_->getVulkanContext()->UpdateSubresource(
            Vulkan_buffer.buffer->getBuffer(), 
            0u, 
            NULL, 
            buffer.getData(), 
            0u, 
            0u
          );
          
          Vulkan_buffer.bound = false;
        }
        if (!Vulkan_buffer.bound)
        {
          Vulkan_buffer.bound = true;
          IVulkanBuffer* buffer = 
            (IVulkanBuffer*)Vulkan_buffer.buffer->getBuffer();
          
          context_->getVulkanContext()->PSSetConstantBuffers(
            (UINT)Vulkan_buffer.slot, 
            1u, 
            &buffer
          );
        }
      }
      for (size_t i = 0u; i < gs_buffers_.size(); ++i)
      {
        auto& buffer = gs_buffers_.at(i);
        auto& Vulkan_buffer = gs_Vulkan_buffers_.at(i);

        if (buffer.getChanged() == true)
        {
          buffer.setChanged(false);
          
          context_->getVulkanContext()->UpdateSubresource(
            Vulkan_buffer.buffer->getBuffer(), 
            0u, 
            NULL, 
            buffer.getData(), 
            0u, 
            0u
          );
          
          Vulkan_buffer.bound = false;
        }
        if (!Vulkan_buffer.bound)
        {
          Vulkan_buffer.bound = true;
          IVulkanBuffer* buffer = 
            (IVulkanBuffer*)Vulkan_buffer.buffer->getBuffer();
          
          context_->getVulkanContext()->GSSetConstantBuffers(
            (UINT)Vulkan_buffer.slot, 
            1u, 
            &buffer
          );
        }
      }*/
    }
    
    ///////////////////////////////////////////////////////////////////////////
		Vector<uint32_t> VulkanShader::getStages() const
    {
      return stages_;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void VulkanShader::updateShaderVariable(
      const platform::ShaderVariable& variable)
    {
      for (size_t i = 0u; i < vs_buffers_.size(); ++i)
      {
        auto& buffer = vs_buffers_.at(i);

        for (auto& v : buffer.getVariables())
        {
          if (variable.name == v.name)
          {
            memcpy(v.data, variable.data.data(), v.size);
            buffer.setChanged(true);
            continue;
          }
        }
      }
      
      for (size_t i = 0u; i < ps_buffers_.size(); ++i)
      {
        auto& buffer = ps_buffers_.at(i);

        for (auto& v : buffer.getVariables())
        {
          if (variable.name == v.name)
          {
            memcpy(v.data, variable.data.data(), v.size);
            buffer.setChanged(true);
            continue;
          }
        }
      }

      for (size_t i = 0u; i < gs_buffers_.size(); ++i)
      {
        auto& buffer = gs_buffers_.at(i);

        for (auto& v : buffer.getVariables())
        {
          if (variable.name == v.name)
          {
            memcpy(v.data, variable.data.data(), v.size);
            buffer.setChanged(true);
            continue;
          }
        }
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    Vector<platform::ShaderBuffer>& VulkanShader::getVsBuffers()
    {
      return vs_buffers_;
    }

    ///////////////////////////////////////////////////////////////////////////
    Vector<platform::ShaderBuffer>& VulkanShader::getPsBuffers()
    {
      return ps_buffers_;
    }

    ///////////////////////////////////////////////////////////////////////////
    Vector<platform::ShaderBuffer>& VulkanShader::getGsBuffers()
    {
      return gs_buffers_;
    }

	///////////////////////////////////////////////////////////////////////////
	Vector<VulkanReflectionInfo> VulkanShader::getTextures()
	{
		return textures_;
	}

	///////////////////////////////////////////////////////////////////////////
	Vector<VulkanReflectionInfo> VulkanShader::getSamplers()
	{
		return samplers_;
	}

	uint32_t VulkanShader::getNumRenderTargets()
	{
		return num_render_targets_;
	}

	///////////////////////////////////////////////////////////////////////////
	void VulkanShader::reflect()
	{
		VkResult result;
		uint32_t resource_count;
		result = vezEnumeratePipelineResources(pipeline_, &resource_count, nullptr);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could reflect on pipeline | %s", vkErrorCode(result));
		Vector<VezPipelineResource> resources(resource_count);
		result = vezEnumeratePipelineResources(pipeline_, &resource_count, resources.data());
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could reflect on pipeline | %s", vkErrorCode(result));

		std::vector<VezPipelineResource> vez_input_layout;
		std::vector<VezPipelineResource> vez_images;
		std::vector<VezPipelineResource> vez_samplers;
		std::vector<VezPipelineResource> vez_output;
		std::vector<VezPipelineResource> vez_uniform_buffers;
		for (const VezPipelineResource& resource : resources)
		{
			switch (resource.resourceType)
			{
			case VezPipelineResourceType::VEZ_PIPELINE_RESOURCE_TYPE_INPUT:
				vez_input_layout.push_back(resource);
				break;
			case VezPipelineResourceType::VEZ_PIPELINE_RESOURCE_TYPE_SAMPLED_IMAGE:
				vez_images.push_back(resource);
				break;
			case VezPipelineResourceType::VEZ_PIPELINE_RESOURCE_TYPE_SAMPLER:
				vez_samplers.push_back(resource);
				break;
			case VezPipelineResourceType::VEZ_PIPELINE_RESOURCE_TYPE_OUTPUT:
				vez_output.push_back(resource);
				break;
			case VezPipelineResourceType::VEZ_PIPELINE_RESOURCE_TYPE_UNIFORM_BUFFER:
				vez_uniform_buffers.push_back(resource);
				break;
			default:
				break;
			}
		}
		
		// Uniform buffers.
		for (const VezPipelineResource& uniform_buffer : vez_uniform_buffers)
		{
			String name = uniform_buffer.name;
			if (name.find("type_") == 0)
				name = name.substr(strlen("type_"));

			Vector<platform::BufferVariable> variables;
			foundation::SharedPtr<void> data = foundation::Memory::makeShared(foundation::Memory::allocate(uniform_buffer.size));
			memset(data.get(), 0, uniform_buffer.size);

			const VezMemberInfo* next_member = uniform_buffer.pMembers;
			while (next_member)
			{
				platform::BufferVariable variable;
				variable.name  = next_member->name;
				variable.count = next_member->arraySize;
				variable.size  = next_member->size;
				variable.data  = (char*)data.get() + next_member->offset;
				variables.push_back(variable);

				next_member = next_member->pNext;
			}

			if ((uniform_buffer.stages & VK_SHADER_STAGE_VERTEX_BIT))
				vs_buffers_.push_back(platform::ShaderBuffer(Name(name), variables, data));
			if ((uniform_buffer.stages & VK_SHADER_STAGE_FRAGMENT_BIT))
				ps_buffers_.push_back(platform::ShaderBuffer(Name(name), variables, data));
			if ((uniform_buffer.stages & VK_SHADER_STAGE_GEOMETRY_BIT))
				gs_buffers_.push_back(platform::ShaderBuffer(Name(name), variables, data));
		}
		
		// Textures.
		for (const VezPipelineResource& resource : vez_images)
		{
			VulkanReflectionInfo refl_info;
			refl_info.name     = resource.name;
			refl_info.set      = resource.set;
			refl_info.binding  = resource.binding;
			if (eastl::find(textures_.begin(), textures_.end(), refl_info) == textures_.end())
				textures_.push_back(refl_info);
		}
		
		// Samplers.
		for (const VezPipelineResource& resource : vez_samplers)
		{
			VulkanReflectionInfo refl_info;
			refl_info.name     = resource.name;
			refl_info.set      = resource.set;
			refl_info.binding  = resource.binding;
			if (eastl::find(samplers_.begin(), samplers_.end(), refl_info) == samplers_.end())
				samplers_.push_back(refl_info);
		}

		// Output.
		for (const VezPipelineResource& resource : vez_output)
		{
			String name = resource.name;
			for (char& ch : name)
				ch = tolower(ch);
			if (name.find("sv_target") != String::npos)
				num_render_targets_++;
		}

		//! Input layout.
		int idx    = 0;
		int offset = 0;
		int inl    = 0;

		struct InputLayout
		{
			int InputSlot;
			String name;
			int InputSlotClass;
			int InstanceDataStepRate;
			int SemanticIndex;
			int AlignedByteOffset;
		};

		Vector<InputLayout> input_layout;
		for (const VezPipelineResource& resource : vez_input_layout)
		{
			String name(resource.name);
			if (name.find("in_var_") == 0)
				name = name.substr(strlen("in_var_"));

			InputLayout element{};

			// Inlining
			if (name.find("inl_") != String::npos)
			{
				name = name.substr(strlen("inl_"));
				element.InputSlot = idx;
				inl++;
			}
			else
			{
				if (inl == 0)
					element.InputSlot = idx++;
				else
					element.InputSlot = ++idx;
				offset = 0;
			}

			element.SemanticIndex = resource.binding;
			element.AlignedByteOffset = offset;
			element.InputSlotClass = 0;
			element.InstanceDataStepRate = 0;

			// Instancing
			if (String(element.name).find("instance_") != String::npos)
			{
				name = name.substr(strlen("instance_"));
				element.InputSlotClass = 1;
				element.InstanceDataStepRate = 1;
			}

			if (inl <= 1)
				stages_.push_back(constexprHash(name));

			input_layout.push_back(element);
		}

	}
  }
}