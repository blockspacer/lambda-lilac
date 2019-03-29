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
		  , pipeline_(VK_NULL_HANDLE)
		  , input_format_(VK_NULL_HANDLE)
	  {
		  auto data = asset::ShaderManager::getInstance()->getData(shader);
		  Vector<VezPipelineShaderStageCreateInfo> shader_stages;

		  VkResult result;

		  if (!data[(int)ShaderStages::kVertex][VIOLET_SPIRV].empty())
		  {
			  VezShaderModuleCreateInfo shader_module_create_info{};
			  shader_module_create_info.pEntryPoint = "VS";
			  shader_module_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
			  shader_module_create_info.codeSize = static_cast<uint32_t>(data[(int)ShaderStages::kVertex][VIOLET_SPIRV].size());
			  shader_module_create_info.pCode = (uint32_t*)data[(int)ShaderStages::kVertex][VIOLET_SPIRV].data();

			  result = vezCreateShaderModule(renderer_->getDevice(), &shader_module_create_info, &vs_);
			  LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not create shader module | %s", vkErrorCode(result));

			  VezPipelineShaderStageCreateInfo shader_stage{};
			  shader_stage.pEntryPoint = "VS";
			  shader_stage.module = vs_;
			  shader_stages.push_back(shader_stage);
		  }

		  if (!data[(int)ShaderStages::kPixel][VIOLET_SPIRV].empty())
		  {
			  VezShaderModuleCreateInfo shader_module_create_info{};
			  shader_module_create_info.pEntryPoint = "PS";
			  shader_module_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			  shader_module_create_info.codeSize = static_cast<uint32_t>(data[(int)ShaderStages::kPixel][VIOLET_SPIRV].size());
			  shader_module_create_info.pCode = reinterpret_cast<const uint32_t*>(data[(int)ShaderStages::kPixel][VIOLET_SPIRV].data());

			  result = vezCreateShaderModule(renderer_->getDevice(), &shader_module_create_info, &ps_);
			  LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not create shader module | %s", vkErrorCode(result));

			  VezPipelineShaderStageCreateInfo shader_stage{};
			  shader_stage.pEntryPoint = "PS";
			  shader_stage.module = ps_;
			  shader_stages.push_back(shader_stage);
		  }

		  if (!data[(int)ShaderStages::kGeometry][VIOLET_SPIRV].empty())
		  {
			  VezShaderModuleCreateInfo shader_module_create_info{};
			  shader_module_create_info.pEntryPoint = "GS";
			  shader_module_create_info.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
			  shader_module_create_info.codeSize = static_cast<uint32_t>(data[(int)ShaderStages::kGeometry][VIOLET_SPIRV].size());
			  shader_module_create_info.pCode = reinterpret_cast<const uint32_t*>(data[(int)ShaderStages::kGeometry][VIOLET_SPIRV].data());

			  result = vezCreateShaderModule(renderer_->getDevice(), &shader_module_create_info, &gs_);
			  LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not create shader module | %s", vkErrorCode(result));

			  VezPipelineShaderStageCreateInfo shader_stage{};
			  shader_stage.pEntryPoint = "GS";
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
		vezDestroyVertexInputFormat(renderer_->getDevice(), input_format_);

      buffers_.resize(0u);
      
      for (auto& buffer : buffers_) 
        renderer_->freeRenderBuffer((platform::IRenderBuffer*&)buffer.buffer);
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanShader::bind()
    {
		vezCmdBindPipeline(pipeline_);
		vezCmdSetVertexInputFormat(input_format_);
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanShader::unbind()
    {
      for (auto& buffer : buffers_)
        buffer.bound = false;
    }

    ///////////////////////////////////////////////////////////////////////////
    void VulkanShader::bindBuffers()
    {
      for (size_t i = 0u; i < buffers_.size(); ++i)
      {
        auto& buffer = buffers_.at(i).shader_buffer;
        auto& gpu_buffer = buffers_.at(i).buffer;

        if (buffer.getChanged() == true)
        {
          buffer.setChanged(false);
		  
		  void* data = gpu_buffer->lock();
		  memcpy(data, buffer.getData(), gpu_buffer->getSize());
		  gpu_buffer->unlock();
          
		  buffers_.at(i).bound = false;
        }

		if (!buffers_[i].bound)
        {
          buffers_[i].bound = true;
		  vezCmdBindBuffer(buffers_[i].buffer->getBuffer(), buffers_[i].offset, VK_WHOLE_SIZE, buffers_[i].set, buffers_[i].binding, 0);
        }
      }
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
      for (size_t i = 0u; i < buffers_.size(); ++i)
      {
        auto& buffer = buffers_.at(i);

        for (auto& v : buffer.shader_buffer.getVariables())
        {
          if (variable.name == v.name)
          {
            memcpy(v.data, variable.data.data(), v.size);
            buffer.shader_buffer.setChanged(true);
            continue;
          }
        }
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    Vector<VulkanBuffer>& VulkanShader::getBuffers()
    {
      return buffers_;
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

			VulkanBuffer buffer;
			buffer.buffer  = (VulkanRenderBuffer*)renderer_->allocRenderBuffer(uniform_buffer.size, platform::IRenderBuffer::kFlagConstant | platform::IRenderBuffer::kFlagDynamic);
			buffer.offset  = uniform_buffer.offset;
			buffer.set     = uniform_buffer.set;
			buffer.binding = uniform_buffer.binding;
			buffer.shader_buffer = platform::ShaderBuffer(Name(name), variables, data);
			buffers_.push_back(buffer);
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

		struct InputLayout
		{
			int location;
			int binding;
			int offset;
			VkFormat format;
		};

		struct InputBuffer
		{
			Vector<InputLayout> input_layouts;
			int stride;
			bool instanced;
		};

		Vector<InputBuffer> input_buffers;

		for (const VezPipelineResource& resource : vez_input_layout)
		{
			if ((resource.stages & VK_SHADER_STAGE_VERTEX_BIT) == 0)
				continue;

			String name(resource.name);
			if (name.find("in_var_") == 0)
				name = name.substr(strlen("in_var_"));

			// Inlining
			if (name.find("inl_") == 0)
			{
				name = name.substr(strlen("inl_"));

				if (input_buffers.empty())
				{
					InputBuffer buffer;
					buffer.instanced = false;
					buffer.stride = 0;
					input_buffers.push_back(buffer);
				}
			}
			else
			{
				InputBuffer buffer;
				buffer.instanced = false;
				buffer.stride = 0;
				input_buffers.push_back(buffer);
			}

			InputBuffer& input_buffer = input_buffers.back();

			if (name.find("instance_") == 0)
			{
				name = name.substr(strlen("instance_"));
				input_buffer.instanced = true;
			}

			InputLayout element;
			element.offset   = input_buffer.stride;
			element.location = (int)input_buffer.input_layouts.size();
			element.binding  = (int)input_buffers.size() - 1;
		
			switch (resource.baseType)
			{
			case VEZ_BASE_TYPE_BOOL:
				switch (resource.vecSize)
				{
				case 1: element.format = VK_FORMAT_R8_SINT; break;
				case 2: element.format = VK_FORMAT_R8G8_SINT; break;
				case 3: element.format = VK_FORMAT_R8G8B8_SINT; break;
				case 4: element.format = VK_FORMAT_R8G8B8A8_SINT; break;
				}
				break;
			case VEZ_BASE_TYPE_CHAR:
				switch (resource.vecSize)
				{
				case 1: element.format = VK_FORMAT_R8_SINT; break;
				case 2: element.format = VK_FORMAT_R8G8_SINT; break;
				case 3: element.format = VK_FORMAT_R8G8B8_SINT; break;
				case 4: element.format = VK_FORMAT_R8G8B8A8_SINT; break;
				}
				break;
			case VEZ_BASE_TYPE_INT:
				switch (resource.vecSize)
				{
				case 1: element.format = VK_FORMAT_R32_SINT; break;
				case 2: element.format = VK_FORMAT_R32G32_SINT; break;
				case 3: element.format = VK_FORMAT_R32G32B32_SINT; break;
				case 4: element.format = VK_FORMAT_R32G32B32A32_SINT; break;
				}
				break;
			case VEZ_BASE_TYPE_UINT:
				switch (resource.vecSize)
				{
				case 1: element.format = VK_FORMAT_R32_UINT; break;
				case 2: element.format = VK_FORMAT_R32G32_UINT; break;
				case 3: element.format = VK_FORMAT_R32G32B32_UINT; break;
				case 4: element.format = VK_FORMAT_R32G32B32A32_UINT; break;
				}
				break;
			case VEZ_BASE_TYPE_UINT64:
				switch (resource.vecSize)
				{
				case 1: element.format = VK_FORMAT_R64_UINT; break;
				case 2: element.format = VK_FORMAT_R64G64_UINT; break;
				case 3: element.format = VK_FORMAT_R64G64B64_UINT; break;
				case 4: element.format = VK_FORMAT_R64G64B64A64_UINT; break;
				}
				break;
			case VEZ_BASE_TYPE_HALF:
				switch (resource.vecSize)
				{
				case 1: element.format = VK_FORMAT_R16_SFLOAT; break;
				case 2: element.format = VK_FORMAT_R16G16_SFLOAT; break;
				case 3: element.format = VK_FORMAT_R16G16B16_SFLOAT; break;
				case 4: element.format = VK_FORMAT_R16G16B16A16_SFLOAT; break;
				}
				break;
			case VEZ_BASE_TYPE_FLOAT:
				switch (resource.vecSize)
				{
				case 1: element.format = VK_FORMAT_R32_SFLOAT; break;
				case 2: element.format = VK_FORMAT_R32G32_SFLOAT; break;
				case 3: element.format = VK_FORMAT_R32G32B32_SFLOAT; break;
				case 4: element.format = VK_FORMAT_R32G32B32A32_SFLOAT; break;
				}
				break;
			case VEZ_BASE_TYPE_DOUBLE:
				switch (resource.vecSize)
				{
				case 1: element.format = VK_FORMAT_R64_SFLOAT; break;
				case 2: element.format = VK_FORMAT_R64G64_SFLOAT; break;
				case 3: element.format = VK_FORMAT_R64G64B64_SFLOAT; break;
				case 4: element.format = VK_FORMAT_R64G64B64A64_SFLOAT; break;
				}
				break;
			}

			switch (resource.baseType)
			{
			case VEZ_BASE_TYPE_BOOL:
			case VEZ_BASE_TYPE_CHAR:
				input_buffer.stride += 1 * resource.vecSize * resource.arraySize;
				break;
			case VEZ_BASE_TYPE_HALF:
				input_buffer.stride += 2 * resource.vecSize * resource.arraySize;
				break;
			case VEZ_BASE_TYPE_INT:
			case VEZ_BASE_TYPE_UINT:
			case VEZ_BASE_TYPE_FLOAT:
				input_buffer.stride += 4 * resource.vecSize * resource.arraySize;
				break;
			case VEZ_BASE_TYPE_UINT64:
			case VEZ_BASE_TYPE_DOUBLE:
				input_buffer.stride += 8 * resource.vecSize * resource.arraySize;
				break;
			}

			input_buffer.input_layouts.push_back(element);

			if (input_buffer.input_layouts.size() == 1)
				stages_.push_back(constexprHash(name));
		}

		Vector<VkVertexInputBindingDescription> vertex_input_binding(input_buffers.size());
		Vector<VkVertexInputAttributeDescription> attribute_descriptions;
		for (uint32_t i = 0; i < input_buffers.size(); ++i)
		{
			vertex_input_binding[i].binding   = i;
			vertex_input_binding[i].inputRate = input_buffers[i].instanced ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
			vertex_input_binding[i].stride    = input_buffers[i].stride;

			for (const auto& input_layout : input_buffers[i].input_layouts)
			{
				VkVertexInputAttributeDescription attribute_description;
				attribute_description.binding  = input_layout.binding;
				attribute_description.format   = input_layout.format;
				attribute_description.location = input_layout.location;
				attribute_description.offset   = input_layout.offset;
				attribute_descriptions.push_back(attribute_description);
			}
		}

		VezVertexInputFormatCreateInfo vertex_input_format_create_info{};
		vertex_input_format_create_info.pVertexAttributeDescriptions    = attribute_descriptions.data();
		vertex_input_format_create_info.vertexAttributeDescriptionCount = (uint32_t)attribute_descriptions.size();
		vertex_input_format_create_info.pVertexBindingDescriptions      = vertex_input_binding.data();
		vertex_input_format_create_info.vertexBindingDescriptionCount   = (uint32_t)vertex_input_binding.size();
		result = vezCreateVertexInputFormat(renderer_->getDevice(), &vertex_input_format_create_info, &input_format_);
		LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could create vertex input format | %s", vkErrorCode(result));
	}
  }
}