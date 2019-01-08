#include "dear_imgui.h"
#include <imgui.h>
#include <utils/file_system.h>
#include "assets/asset_manager.h"
#include "input/keyboard.h"

#ifndef DEAR_IMGUI_MAX_VERTICES
#define DEAR_IMGUI_MAX_VERTICES 512 * 1024
#endif
#ifndef DEAR_IMGUI_MAX_INDICES 
#define DEAR_IMGUI_MAX_INDICES  128 * 1024
#endif

namespace lambda
{
  void* dearImGUIMalloc(size_t size, void* user_data)
  {
    return foundation::Memory::allocate(size);
  }
  void dearImGUIFree(void* ptr, void* user_data)
  {
    if (ptr != nullptr)
    {
      foundation::Memory::deallocate(ptr);
    }
  }

  namespace imgui
  {
    size_t DearImGUI::k_instance_count_ = 0u;

    void DearImGUI::setWindow(foundation::SharedPointer<platform::IWindow> window)
    {
      window_ = window;

      ImGuiIO& io = ImGui::GetIO();
      io.DisplaySize.x = (float)window_->getSize().x;
      io.DisplaySize.y = (float)window_->getSize().y;
    }

    void DearImGUI::initialize()
    {
      k_instance_count_++;

      ImGui::SetAllocatorFunctions(dearImGUIMalloc, dearImGUIFree);
      context_ = ImGui::CreateContext();
      ImGui::StyleColorsDark();

      max_vertex_count_ = DEAR_IMGUI_MAX_VERTICES / sizeof(platform::ImGUIVertex);
      max_index_count_  = DEAR_IMGUI_MAX_INDICES / sizeof(uint16_t);
      data_vertices_    = (platform::ImGUIVertex*)foundation::Memory::allocate(DEAR_IMGUI_MAX_VERTICES);
      data_indices_     = (uint16_t*)foundation::Memory::allocate(DEAR_IMGUI_MAX_INDICES);

      command_list_.mesh = asset::AssetManager::getInstance().createAsset(
        "__dear_imgui_mesh" + toString(k_instance_count_) + "__",
        foundation::Memory::constructShared<asset::Mesh>()
      );

      String bytecode = R"(
cbuffer buffer0 : register(b0)
{
  float4x4 projection_matrix;
};

Texture2D    tex : register(t0);
SamplerState sam : register(s0);

struct VS_INPUT
{
  float3 pos : INL_POSITION;
  float2 uv  : INL_TxEX_COORD;
  float4 col : INL_CxOLOUR;
};

struct PS_INPUT
{
  float4 pos : SV_POSITION;
  float2 uv  : TEXCOORD0;
  float4 col : COLOR0;
};

PS_INPUT VS(VS_INPUT input)
{
  PS_INPUT output;
  output.pos = mul(projection_matrix, float4(input.pos.xy, 0.f, 1.f));
  output.col = input.col;
  output.uv  = input.uv;
  return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
  return input.col * tex.Sample(sam, input.uv);
})";

      Vector<char> bytecode_vec(bytecode.begin(), bytecode.end());

      command_list_.shader = asset::AssetManager::getInstance().createAsset(
        "__dear_imgui_shader" + toString(k_instance_count_) + "__",
        foundation::Memory::constructShared<asset::Shader>(Name("imgui"), bytecode_vec)
      );

      ImGuiIO& io = ImGui::GetIO();
      io.KeyMap[ImGuiKey_Tab] = (int)io::KeyboardKeys::kTab;
      io.KeyMap[ImGuiKey_LeftArrow] = (int)io::KeyboardKeys::kLeft;
      io.KeyMap[ImGuiKey_RightArrow] = (int)io::KeyboardKeys::kRight;
      io.KeyMap[ImGuiKey_UpArrow] = (int)io::KeyboardKeys::kUp;
      io.KeyMap[ImGuiKey_DownArrow] = (int)io::KeyboardKeys::kDown;
      io.KeyMap[ImGuiKey_Backspace] = (int)io::KeyboardKeys::kBackspace;
      io.KeyMap[ImGuiKey_Space] = (int)io::KeyboardKeys::kSpace;
      io.KeyMap[ImGuiKey_Enter] = (int)io::KeyboardKeys::kReturn;
      io.KeyMap[ImGuiKey_A] = 'A';
      io.KeyMap[ImGuiKey_C] = 'C';
      io.KeyMap[ImGuiKey_V] = 'V';
      io.KeyMap[ImGuiKey_X] = 'X';
      io.KeyMap[ImGuiKey_Y] = 'Y';
      io.KeyMap[ImGuiKey_Z] = 'Z';
      io.IniFilename = nullptr;
    }
    void DearImGUI::update(const double& delta_time)
    {
      /*if (ImGui::BeginMainMenuBar())
      {
        if (ImGui::BeginMenu("Menu"))
        {
          imButton("test_button", glm::vec2(50.0f, 20.0f));
          imImage(default_font_, glm::vec2(50.0f));
          ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Menu2"))
        {
          ImGui::MenuItem("m1", NULL);
          ImGui::MenuItem("m2", NULL);
          ImGui::MenuItem("m3", NULL);
          ImGui::MenuItem("m4", NULL);
          ImGui::MenuItem("m5", NULL);
          ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Menu3"))
        {
          ImGui::MenuItem("m1", NULL);
          ImGui::MenuItem("m2", NULL);
          ImGui::MenuItem("m3", NULL);
          ImGui::MenuItem("m4", NULL);
          ImGui::MenuItem("m5", NULL);
          ImGui::EndMenu();
        }
      }
      ImGui::EndMainMenuBar();*/

    }
    void DearImGUI::inputStart()
    {
    }
    bool DearImGUI::inputHandleMessage(const platform::WindowMessage& message)
    {
      ImGuiIO& io = ImGui::GetIO();
      
      if (false == io.WantCaptureMouse)
      {
        if (message.type == platform::WindowMessageType::kMouseMove)
        {
          io.MousePos.x = (float)message.data[0];
          io.MousePos.y = (float)message.data[1];
        }
        return false;
      }

      switch(message.type)
      {
      case platform::WindowMessageType::kMouseButton:
      {
        if (message.data[0] <= 3)
        {
          io.MouseDown[(int)message.data[0]] = (int)message.data[1] > 0 ? true : false;
          break;
        }
        else
        {
          return false;
        }
        break;
      }
      case platform::WindowMessageType::kMouseMove:
      {
        io.MousePos.x = (float)message.data[0];
        io.MousePos.y = (float)message.data[1];
        break;
      }
      case platform::WindowMessageType::kMouseScroll:
      {
        io.MouseWheel += (float)message.data[0];
        break;
      }
      case platform::WindowMessageType::kKeyboardButton:
      {
        if (message.data[0] < 256u)
        {
          io.KeysDown[message.data[0]] = message.data[1] > 0 ? true : false;
        }
        break;
      }
      case platform::WindowMessageType::kChar:
      {
        io.AddInputCharacter((char)message.data[0]);
        break;
      }
      case platform::WindowMessageType::kKeyboardModifier:
        break;
      default:
        return false;
        break;
      }
      return true;
    }
    void DearImGUI::inputEnd()
    {
    }
    void DearImGUI::setFont(const String& file_path, float font_size)
    {
      ImGuiIO& io = ImGui::GetIO();

      ImFontConfig config;
      config.FontDataOwnedByAtlas = false;
      config.MergeMode            = false;
      config.OversampleH          = 1;
      config.OversampleV          = 1;

      Vector<char> file = FileSystem::FileToVector(file_path);
      io.Fonts->AddFontFromMemoryTTF(file.data(), (int)file.size(), font_size,&config);

      unsigned char* pixels;
      int width;
      int height;
      int bpp;
      io.Fonts->GetTexDataAsRGBA32(&pixels,&width,&height,&bpp);

      assert(bpp == 4);

      // Gather the data for the texture.
      glm::uvec2 size(width, height);
      Vector<unsigned char> data(size.x * size.y * bpp);
      memcpy(data.data(), pixels, data.size());

      // Store the font.
      asset::TextureManager::getInstance()->create(
        Name("__dear_imgui_font" + toString(k_instance_count_) + "__"), 
        size.x, 
        size.y, 
        1u, 
        TextureFormat::kR8G8B8A8, 
        0u, 
        data
      );

      io.Fonts->TexID = (void*)&default_font_;

      // HACK
      startFrame();
    }

    void DearImGUI::endFrame()
    {
      ImGui::EndFrame();
    }

    void DearImGUI::startFrame()
    {
      textures_.clear();
      ImGui::NewFrame();
    }

    glm::mat4 getProjectionMatrix2(unsigned int width, unsigned int height)
    {
      const float L = 0.0f;
      const float R = (float)width;
      const float T = 0.0f;
      const float B = (float)height;
      return glm::mat4(
        2.0f / (R - L),    0.0f,              0.0f, 0.0f,
        0.0f,              2.0f / (T - B),    0.0f, 0.0f,
        0.0f,              0.0f,              0.5f, 0.0f,
        (R + L) / (L - R), (T + B) / (B - T), 0.5f, 1.0f
      );
    }

    void DearImGUI::generateCommandList()
    {
      ImGui::Render();
      ImDrawData* draw_data = ImGui::GetDrawData();
      ImGuiIO& io = ImGui::GetIO();
      io.DisplaySize.x = (float)window_->getSize().x;
      io.DisplaySize.y = (float)window_->getSize().y;

      // Update the vertex and index buffer.
      platform::ImGUIVertex* vtx_dst = data_vertices_;
      ImDrawIdx*  idx_dst = (ImDrawIdx*)data_indices_;
      for (int n = 0; n < draw_data->CmdListsCount; n++)
      {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        
        // Vertex
        for (int i = 0u; i < cmd_list->VtxBuffer.Size; ++i)
        {
          ImDrawVert* imgui_vertex = cmd_list->VtxBuffer.Data + i;
          platform::ImGUIVertex* vertex = vtx_dst + i;
          ImVec4 col = ImGui::ColorConvertU32ToFloat4(imgui_vertex->col);
          memcpy(&vertex->position,&imgui_vertex->pos, sizeof(ImDrawVert::pos));
          memcpy(&vertex->uv,     &imgui_vertex->uv,  sizeof(ImDrawVert::uv));
          memcpy(&vertex->colour,&col, sizeof(ImVec4));
        }
        vtx_dst += cmd_list->VtxBuffer.Size;
        
        // Index
        memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        idx_dst += cmd_list->IdxBuffer.Size;
      }

      command_list_.projection = getProjectionMatrix2((unsigned int)io.DisplaySize.x, (unsigned int)io.DisplaySize.y);
      command_list_.commands.resize(0u);
      Vector<asset::SubMesh> sub_meshes;
      Vector<asset::VioletTextureHandle> textures;

      int vtx_offset = 0;
      int idx_offset = 0;
      ImVec2 pos = draw_data->DisplayPos;
      for (int n = 0; n < draw_data->CmdListsCount; n++)
      {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
          const ImDrawCmd* pcmd =&cmd_list->CmdBuffer[cmd_i];
          if (pcmd->UserCallback)
          {
            // User callback (registered via ImDrawList::AddCallback)
            pcmd->UserCallback(cmd_list, pcmd);
          }
          else
          {
            // Apply scissor/clipping rectangle
            platform::ImGUICommand imgui_command;
            imgui_command.scissor_rect = glm::vec4(
              float(pcmd->ClipRect.x - pos.x),
              float(pcmd->ClipRect.y - pos.y),
              float(pcmd->ClipRect.z - pos.x),
              float(pcmd->ClipRect.w - pos.y)
            );
            imgui_command.scissor_rect.z -= imgui_command.scissor_rect.x;
            imgui_command.scissor_rect.w -= imgui_command.scissor_rect.y;
            command_list_.commands.push_back(imgui_command);

            asset::VioletTextureHandle* texture = (asset::VioletTextureHandle*)pcmd->TextureId;
            if (texture != nullptr)
            {
              textures.push_back(*texture);
            }
            else
            {
              textures.push_back(asset::VioletTextureHandle());
            }

            asset::SubMesh sub_mesh;
            sub_mesh.offset.at(asset::MeshElements::kPositions) = asset::SubMesh::Offset(0u, max_vertex_count_, sizeof(platform::ImGUIVertex));
            sub_mesh.offset.at(asset::MeshElements::kIndices)   = asset::SubMesh::Offset(0u, pcmd->ElemCount,   sizeof(uint32_t));
            sub_mesh.io.tex_alb    = (int)textures.size() - 1;
            sub_mesh.index_offset  = idx_offset;
            sub_mesh.vertex_offset = vtx_offset;
            sub_mesh.io.double_sided = true;
            sub_meshes.push_back(sub_mesh);
          }
          idx_offset += pcmd->ElemCount;
        }
        vtx_offset += cmd_list->VtxBuffer.Size;
      }

      if(command_list_.commands.size() > 0u)
      {
        // Set the new vertex, index buffers, sub meshes, and textures.
        command_list_.mesh->set(asset::MeshElements::kPositions, asset::Mesh::Buffer(data_vertices_, (uint32_t)max_vertex_count_, sizeof(platform::ImGUIVertex)));
        command_list_.mesh->set(asset::MeshElements::kIndices,   asset::Mesh::Buffer(data_indices_,  (uint32_t)max_index_count_,  sizeof(uint16_t)));
        command_list_.mesh->setSubMeshes(sub_meshes);
        command_list_.mesh->setAttachedTextureCount(glm::uvec3(textures.size(), 0u, 0u));
        command_list_.mesh->setAttachedTextures(textures);
      }
      sub_meshes.clear();
      textures.clear();
    }
    void DearImGUI::deinitialize()
    {
      endFrame();
      ImGui::DestroyContext(context_);

      command_list_.mesh->clear();

      foundation::Memory::deallocate(data_vertices_);
      foundation::Memory::deallocate(data_indices_);
    }

    ImVec2 toImVec2(const glm::vec2& v)
    {
      return ImVec2(v.x, v.y);
    }
    ImVec4 toImVec4(const glm::vec4& v)
    {
      return ImVec4(v.x, v.y, v.z, v.w);
    }

    bool DearImGUI::imBegin(const String& name, bool& is_open, const glm::vec2& position, const glm::vec2& size, const uint8_t& flags)
    {
      uint32_t flgs = 0;
      if ((flags& platform::ImGUIFlags::kMovable)     == 0) flgs |= ImGuiWindowFlags_::ImGuiWindowFlags_NoMove;
      if ((flags& platform::ImGUIFlags::kScalable)    == 0) flgs |= ImGuiWindowFlags_::ImGuiWindowFlags_NoResize;
      if ((flags& platform::ImGUIFlags::kClosable)     > 0) flgs |= 0;
      if ((flags& platform::ImGUIFlags::kMinimizable) == 0) flgs |= ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse;
      if ((flags& platform::ImGUIFlags::kTitle)       == 0) flgs |= ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar;
      if ((flags& platform::ImGUIFlags::kNoInput)      > 0) flgs |= ImGuiWindowFlags_::ImGuiWindowFlags_NoInputs;
      if ((flags& platform::ImGUIFlags::kNoScrollbar)  > 0) flgs |= ImGuiWindowFlags_::ImGuiWindowFlags_NoScrollbar;
      
      ImGui::Begin(name.c_str(),&is_open, flgs);
      ImGui::SetWindowPos(toImVec2(position), ((flags& platform::ImGUIFlags::kMovable)  > 0) ? ImGuiCond_Once : ImGuiCond_Always);
      ImGui::SetWindowSize(toImVec2(size),    ((flags& platform::ImGUIFlags::kScalable) > 0) ? ImGuiCond_Once : ImGuiCond_Always);
      return is_open;
    }
    void DearImGUI::imEnd()
    {
      ImGui::End();
    }
    void DearImGUI::imText(const String& text, const platform::ImGUITextAlign& text_align)
    {
      ImGui::Text(text.c_str());
    }
    void DearImGUI::imTextMultiLine(const String& text, const platform::ImGUITextAlign& text_align)
    {
      for (const String& line : split(text, '\n'))
      {
        imText(line);
      }
    }
    void DearImGUI::imTextColoured(const String& text, const glm::vec4& colour, const platform::ImGUITextAlign& text_align)
    {
      ImGui::TextColored(toImVec4(colour), text.c_str());
    }
    void DearImGUI::imLabel(const String& label, const platform::ImGUITextAlign& text_align)
    {
      pushId(label);
      ImGui::LabelText("", label.c_str());
      ImGui::PopID();
    }
    bool DearImGUI::imButton(const String& label, const glm::vec2& size)
    {
      return ImGui::Button(label.c_str(), toImVec2(size));
    }
    bool DearImGUI::imButtonImage(const asset::VioletTextureHandle& texture, const glm::vec2& size)
    {
      if (textures_.find(texture.get()) == textures_.end())
      {
        textures_.insert(eastl::make_pair(texture.get(), texture));
      }

      pushId("", (void*)&texture);
      bool ret = ImGui::ImageButton((void*)&textures_.at(texture.get()), ImVec2(size.x, size.y));
      ImGui::PopID();

      return ret;
    }
    bool DearImGUI::imColourPicker(const String& label, glm::vec4& colour, const platform::ImGUIColour& colour_type, const platform::ImGUITextAlign& text_align)
    {
      ImGui::Text(label.c_str());
      ImGui::SameLine();
      uint32_t flags = colour_type == platform::ImGUIColour::kHSV ? ImGuiColorEditFlags_::ImGuiColorEditFlags_RGB : ImGuiColorEditFlags_::ImGuiColorEditFlags_HSV;
      flags |= ImGuiColorEditFlags_::ImGuiColorEditFlags_AlphaPreview;
      flags |= ImGuiColorEditFlags_::ImGuiColorEditFlags_NoLabel;
      flags |= ImGuiColorEditFlags_::ImGuiColorEditFlags_NoInputs;

      if (colour_type == platform::ImGUIColour::kHSV)
      {
        ImGui::ColorConvertHSVtoRGB(colour.x, colour.y, colour.z, colour.x, colour.y, colour.z);
      }

      pushId(label);
      bool ret = ImGui::ColorEdit4("",&colour.x, flags);
      ImGui::PopID();
      
      if (colour_type == platform::ImGUIColour::kHSV)
      {
        ImGui::ColorConvertRGBtoHSV(colour.x, colour.y, colour.z, colour.x, colour.y, colour.z);
      }

      return ret;
    }
    bool DearImGUI::imTextEdit(const String& label, String& text)
    {
      static const int k_buffer_size = 255;
      static char k_buffer[k_buffer_size];

      // Update buffer.
      memcpy(k_buffer, text.data(), text.size());

      // Text edit.
      ImGui::Text(label.c_str());
      ImGui::SameLine();
      pushId(label);
      bool ret = ImGui::InputText("", k_buffer, k_buffer_size);
      ImGui::PopID();

      // Update text.
      if (ret == true)
      {
        text.resize(k_buffer_size);
        memcpy((void*)text.data(), k_buffer, k_buffer_size);
        text.erase((eastl::find(text.begin(), text.end(), '\0')), text.end());
      }

      return ret;
    }
    bool DearImGUI::imFloat1(const String& label, float& val)
    {
      ImGui::Text(label.c_str());
      ImGui::SameLine();
      pushId(label,&val);
      bool ret = ImGui::InputFloat("",&val);
      ImGui::PopID();
      return ret;
    }
    bool DearImGUI::imFloat2(const String& label, glm::vec2& val)
    {
      ImGui::Text(label.c_str());
      ImGui::SameLine();
      pushId(label,&val);
      bool ret = ImGui::InputFloat2("",&val.x);
      ImGui::PopID();
      return ret;
    }
    bool DearImGUI::imFloat3(const String& label, glm::vec3& val)
    {
      ImGui::Text(label.c_str());
      ImGui::SameLine();
      pushId(label,&val);
      bool ret = ImGui::InputFloat3("",&val.x);
      ImGui::PopID();
      return ret;
    }
    bool DearImGUI::imFloat4(const String& label, glm::vec4& val)
    {
      ImGui::Text(label.c_str());
      ImGui::SameLine();
      pushId(label,&val);
      bool ret = ImGui::InputFloat4("",&val.x);
      ImGui::PopID();
      return ret;
    }
    void DearImGUI::imImage(const asset::VioletTextureHandle& texture, const glm::vec2& size)
    {
      if (textures_.find(texture.get()) == textures_.end())
      {
        textures_.insert(eastl::make_pair(texture.get(), texture));
      }

      ImGui::Image((void*)&textures_.at(texture.get()), ImVec2(size.x, size.y));
    }
    bool DearImGUI::imMainMenuBarBegin(const uint8_t& menu_items)
    {
      menu_bar_open_ = ImGui::BeginMainMenuBar();
      return menu_bar_open_;
    }
    void DearImGUI::imMainMenuBarEnd()
    {
      ImGui::EndMainMenuBar();
    }
    bool DearImGUI::imMenuBegin(const String& label, const glm::vec2& size)
    {
      menu_open_ = ImGui::BeginMenu(label.c_str());
      return menu_open_;
    }
    void DearImGUI::imMenuEnd()
    {
      if (menu_open_)
      {
        ImGui::EndMenu();
      }
    }
    void DearImGUI::pushId(const String& label)
    {
      ImGui::PushID((int)hash(label));
    }
    void DearImGUI::pushId(const String& label, void* val)
    {
      ImGui::PushID((int)(hash(label) + intptr_t(&val)));
    }
  }
}