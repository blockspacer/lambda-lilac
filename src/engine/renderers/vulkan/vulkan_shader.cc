#include "vulkan_shader.h"
#include <utils/file_system.h>
#include "vulkan_renderer.h"

namespace lambda
{
  namespace linux
  {
		bool compile(VkDevice device, const char* data, uint32_t size, VkShaderModule& module, VezPipelineShaderStageCreateInfo& shader_stage, const char* entry, VkShaderStageFlagBits stage)
		{
			VezShaderModuleCreateInfo shader_module_create_info{};
			shader_module_create_info.pEntryPoint = entry;
			shader_module_create_info.stage       = stage;
			shader_module_create_info.codeSize    = size;
			shader_module_create_info.pCode       = (uint32_t*)data;

			VkResult result;
			result = vezCreateShaderModule(device, &shader_module_create_info, &module);
			LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not create shader module | %s", vkErrorCode(result));
			if (result != VK_SUCCESS && module != VK_NULL_HANDLE)
			{
				// If shader module creation failed but error is from GLSL compilation, get the error log.
				uint32_t info_log_size = 0;
				vezGetShaderModuleInfoLog(module, &info_log_size, nullptr);
				String info_log(info_log_size, '\0');
				vezGetShaderModuleInfoLog(module, &info_log_size, &info_log[0]);
				vezDestroyShaderModule(device, module);
				foundation::Error(info_log + "\n");
				return false;
			}

			shader_stage.pEntryPoint = entry;
			shader_stage.module      = module;

			return true;
		}

    ///////////////////////////////////////////////////////////////////////////
#pragma optimize("", off)
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
				VezPipelineShaderStageCreateInfo shader_stage{};
				if (compile(
					renderer_->getDevice(),
					(char*)data[(int)ShaderStages::kVertex][VIOLET_SPIRV].data(),
					(uint32_t)data[(int)ShaderStages::kVertex][VIOLET_SPIRV].size(),
					vs_,
					shader_stage,
					"VS",
					VK_SHADER_STAGE_VERTEX_BIT))
				{
					shader_stages.push_back(shader_stage);
				}
		  }

		  if (!data[(int)ShaderStages::kPixel][VIOLET_SPIRV].empty())
		  {
				VezPipelineShaderStageCreateInfo shader_stage{};
				if (compile(
					renderer_->getDevice(),
					(char*)data[(int)ShaderStages::kPixel][VIOLET_SPIRV].data(),
					(uint32_t)data[(int)ShaderStages::kPixel][VIOLET_SPIRV].size(),
					ps_,
					shader_stage,
					"PS",
					VK_SHADER_STAGE_FRAGMENT_BIT))
				{
					shader_stages.push_back(shader_stage);
				}
		  }

		  if (!data[(int)ShaderStages::kGeometry][VIOLET_SPIRV].empty())
		  {
				VezPipelineShaderStageCreateInfo shader_stage{};
				if (compile(
					renderer_->getDevice(),
					(char*)data[(int)ShaderStages::kGeometry][VIOLET_SPIRV].data(),
					(uint32_t)data[(int)ShaderStages::kGeometry][VIOLET_SPIRV].size(),
					gs_,
					shader_stage,
					"GS",
					VK_SHADER_STAGE_GEOMETRY_BIT))
				{
					shader_stages.push_back(shader_stage);
				}
		  }

		   
		  VezGraphicsPipelineCreateInfo pipeline_create_info{};
		  pipeline_create_info.stageCount = static_cast<uint32_t>(shader_stages.size());
		  pipeline_create_info.pStages = shader_stages.data();
		  result = vezCreateGraphicsPipeline(renderer_->getDevice(), &pipeline_create_info, &pipeline_);
		  LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could not create pipeline | %s", vkErrorCode(result));

		  // Reflection.
		  for (uint32_t i = 0; i < (uint32_t)ShaderStages::kCount; ++i)
		  {
			  for (const auto& resource : shader->getResources((ShaderStages)i))
			  {
				  VulkanReflectionInfo info{};
				  switch (resource.type)
				  {
				  case VioletShaderResourceType::kConstantBuffer:
				  {
					  Vector<platform::BufferVariable> variables;
					  foundation::SharedPtr<void> data = foundation::Memory::makeShared(foundation::Memory::allocate(resource.size));
					  memset(data.get(), 0, resource.size);

					  for (const auto& item : resource.items)
					  {
						  platform::BufferVariable variable;
						  variable.name = item.name;
						  variable.count = 0;
						  variable.size = item.size;
						  variable.data = (char*)data.get() + item.offset;
						  variables.push_back(variable);
					  }

					  VulkanBuffer buffer;
					  buffer.buffer  = (VulkanRenderBuffer*)renderer_->allocRenderBuffer(resource.size, platform::IRenderBuffer::kFlagConstant | platform::IRenderBuffer::kFlagDynamic);
					  buffer.offset  = 0;
					  buffer.set     = 0;
					  buffer.binding = resource.slot;
					  buffer.slot    = resource.slot - 0;
					  buffer.shader_buffer = platform::ShaderBuffer(Name(resource.name), variables, data);
					  buffers_.push_back(buffer);
					  break;
				  }
				  case VioletShaderResourceType::kSampler:
					  info.slot = resource.slot;
					  info.binding = info.slot + 200;
					  info.set = 0;
					  info.name = resource.name;
					  samplers_.push_back(info);
					  break;
				  case VioletShaderResourceType::kTexture:
					  info.slot = resource.slot;
					  info.binding = info.slot + 100;
					  info.set = 0;
					  info.name = resource.name;
					  textures_.push_back(info);
					  break;
				  }
			  }
		  }

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
#pragma optimize ("", off)
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

				//if (!buffers_[i].bound)
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
#pragma optimize("", off)
		void VulkanShader::reflect()
		{
			VkResult result;
			uint32_t resource_count;
			result = vezEnumeratePipelineResources(pipeline_, &resource_count, nullptr);
			LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could reflect on pipeline | %s", vkErrorCode(result));
			Vector<VezPipelineResource> resources(resource_count);
			result = vezEnumeratePipelineResources(pipeline_, &resource_count, resources.data());
			LMB_ASSERT(result == VK_SUCCESS, "VULKAN: Could reflect on pipeline | %s", vkErrorCode(result));

			Vector<VezPipelineResource> vez_input_layout;
			Vector<VezPipelineResource> vez_output;
			for (const VezPipelineResource& resource : resources)
			{
				switch (resource.resourceType)
				{
				case VezPipelineResourceType::VEZ_PIPELINE_RESOURCE_TYPE_INPUT:

					if ((resource.stages & VK_SHADER_STAGE_VERTEX_BIT))
						vez_input_layout.push_back(resource);
					break;
				case VezPipelineResourceType::VEZ_PIPELINE_RESOURCE_TYPE_OUTPUT:
					vez_output.push_back(resource);
					break;
				default:
					break;
				}
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
				uint32_t hash;
				bool use_me;
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
				String name(resource.name);
				if (name.find("in_var_") == 0)
					name = name.substr(strlen("in_var_"));
				InputLayout element;
				element.use_me = false;

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

				// Inlining
				if (name.find("use_me_") == 0)
				{
					name = name.substr(strlen("use_me_"));
					element.use_me = true;
				}

				InputBuffer& input_buffer = input_buffers.back();

				if (name.find("instance_") == 0)
				{
					name = name.substr(strlen("instance_"));
					input_buffer.instanced = true;
				}

				element.offset   = input_buffer.stride;
				element.location = resource.location;// (int)input_buffer.input_layouts.size();
				element.binding  = resource.binding;// (int)input_buffers.size() - 1;
				element.hash     = constexprHash(name);

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
			}

			for (const InputBuffer& input_buffer : input_buffers)
			{
				uint32_t hash = 0u;
				int best = INT_MAX;

				for (const InputLayout& input_layout : input_buffer.input_layouts)
				{
					if (input_layout.use_me)
					{
						hash = input_layout.hash;
						break;
					}
					if (input_layout.offset < best)
					{
						best = input_layout.offset;
						hash = input_layout.hash;
					}
				}
				stages_.push_back(hash);
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