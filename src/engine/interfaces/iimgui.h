#pragma once
#include <memory/memory.h>
#include "assets/mesh.h"
#include "assets/shader.h"

namespace lambda
{
  namespace world
  {
    class IWorld;
  }

  namespace platform
  {
    ///////////////////////////////////////////////////////////////////////////
    struct ImGUIVertex
    {
      glm::vec3 position;
      glm::vec2 uv;
      glm::vec4 colour;
    };
    
    ///////////////////////////////////////////////////////////////////////////
    struct ImGUICommand
    {
      glm::vec4 scissor_rect;
    };
   
    ///////////////////////////////////////////////////////////////////////////
    struct ImGUICommandList
    {
      asset::ShaderHandle shader;
      asset::MeshHandle mesh;
      Vector<ImGUICommand> commands;
      glm::mat4 projection;
    };

    class IWindow;
    struct WindowMessage;

    ///////////////////////////////////////////////////////////////////////////
    enum class ImGUITextAlign : unsigned char
    {
      kLeft,
      kCenter,
      kRight
    };

    ///////////////////////////////////////////////////////////////////////////
    enum class ImGUIColour : unsigned char
    {
      kRGB,
      kHSV
    };

    ///////////////////////////////////////////////////////////////////////////
    enum ImGUIFlags : uint8_t
    {
      kMovable     = 1 << 1,
      kScalable    = 1 << 2,
      kClosable    = 1 << 3,
      kMinimizable = 1 << 4,
      kTitle       = 1 << 5,
      kNoInput     = 1 << 6,
      kNoScrollbar = 1 << 7,
    };

    ///////////////////////////////////////////////////////////////////////////
    class IImGUI
    {
    public:
      virtual String name() const = 0;
      virtual void initialize(world::IWorld* world) = 0;
      virtual void update(const double& delta_time) = 0;
      virtual void deinitialize() = 0;
      virtual void inputStart() = 0;
      virtual bool inputHandleMessage(const WindowMessage& message) = 0;
      virtual void inputEnd() = 0;
      virtual void setFont(const String& file_path, float font_size) = 0;
      virtual void endFrame() = 0;
      virtual void startFrame() = 0;
      virtual void generateCommandList() = 0;
      virtual ImGUICommandList getCommandList() = 0;

      /**
      * @brief This functions begins the window creation process. 
      * All ImGUI functions that are called between this and imEnd() get 
      * associated with this window.
      * @param[in] is_open (bool&) Wether the window is open, 
      * gets updated by the ImGUI.
      * @param[in] name (String&) The window name.
      * @param[in] position (const glm::vec2&) The position of the window.
      * @param[in] size (const glm::vec2&) The size of the window.
      * @param[in] flags (uint32_t) The window flags. NOT IMPLEMENTED
      * @return (bool) Returns is_open.
      */
      virtual bool imBegin(
        const String& name, 
        bool& is_open, 
        const glm::vec2& position, 
        const glm::vec2& size, 
        const uint8_t& flags = ImGUIFlags::kTitle | ImGUIFlags::kMinimizable
      ) = 0;
      virtual void imEnd() = 0;
      virtual void imText(
        const String& text, 
        const ImGUITextAlign& text_align = ImGUITextAlign::kLeft
      ) = 0;
      virtual void imTextColoured(
        const String& text, 
        const glm::vec4& colour, 
        const ImGUITextAlign& text_align = ImGUITextAlign::kLeft
      ) = 0;
      virtual void imTextMultiLine(
        const String& text, 
        const ImGUITextAlign& text_align = ImGUITextAlign::kLeft
      ) = 0;
      virtual void imLabel(
        const String& label, 
        const ImGUITextAlign& text_align = ImGUITextAlign::kLeft
      ) = 0;
      virtual bool imButton(const String& label, const glm::vec2& size) = 0;
      virtual bool imButtonImage(
        const asset::VioletTextureHandle& texture, 
        const glm::vec2& size
      ) = 0;
      virtual bool imColourPicker(
        const String& label, 
        glm::vec4& colour, 
        const ImGUIColour& colour_type, 
        const ImGUITextAlign& text_align = ImGUITextAlign::kLeft
      ) = 0;
      virtual bool imTextEdit(const String& label, String& text) = 0;
      virtual bool imFloat1(const String& label, float& val) = 0;
      virtual bool imFloat2(const String& label, glm::vec2& val) = 0;
      virtual bool imFloat3(const String& label, glm::vec3& val) = 0;
      virtual bool imFloat4(const String& label, glm::vec4& val) = 0;
      virtual void imImage(
        const asset::VioletTextureHandle& texture, 
        const glm::vec2& size
      ) = 0;

      virtual bool imMainMenuBarBegin(const uint8_t& menu_items) = 0;
      virtual void imMainMenuBarEnd() = 0;
      virtual bool imMenuBegin(const String& label, const glm::vec2& size) = 0;
      virtual void imMenuEnd() = 0;
    };
  }
}
