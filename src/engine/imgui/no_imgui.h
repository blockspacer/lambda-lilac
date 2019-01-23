#pragma once
#include "interfaces/iimgui.h"

namespace lambda
{
  namespace imgui
  {
    class NoImGUI : public platform::IImGUI
    {
    public:
      virtual ~NoImGUI() {}
      inline virtual String name() const override { return "no imgui"; };
      inline virtual void initialize(world::IWorld* /*world*/) override {};
      inline virtual void update(const double& /*delta_time*/) override {};
      inline virtual void deinitialize() override {};
      inline virtual void inputStart() override {};
      inline virtual bool inputHandleMessage(const platform::WindowMessage& /*message*/) override { return false; };
      inline virtual void inputEnd() override {};
      inline virtual void setFont(const String& /*file_path*/, float /*font_size*/) override {};
      inline virtual void endFrame() override {};
      inline virtual void startFrame() override {};
      inline virtual void generateCommandList() override {};
      inline virtual platform::ImGUICommandList getCommandList() override { return platform::ImGUICommandList(); };

      inline virtual bool imBegin(const String& /*name*/, bool& /*is_open*/, const glm::vec2& /*position*/, const glm::vec2& /*size*/, const uint8_t& /*flags = 0u*/) override { return false; };
      inline virtual void imEnd() override {};
      inline virtual void imText(const String& /*text*/, const platform::ImGUITextAlign& /*text_align = platform::ImGUITextAlign::kLeft*/) override {};
      inline virtual void imTextColoured(const String& /*text*/, const glm::vec4& /*colour*/, const platform::ImGUITextAlign& /*text_align = platform::ImGUITextAlign::kLeft*/) override {};
      inline virtual void imTextMultiLine(const String& /*text*/, const platform::ImGUITextAlign& /*text_align = platform::ImGUITextAlign::kLeft*/) override {};
      inline virtual void imLabel(const String& /*label*/, const platform::ImGUITextAlign& /*text_align = platform::ImGUITextAlign::kLeft*/) override {};
      inline virtual bool imButton(const String& /*label*/, const glm::vec2& /*size*/) override { return false; };
      inline virtual bool imButtonImage(const asset::VioletTextureHandle& /*texture*/, const glm::vec2& /*size*/) override { return false;  };
      inline virtual bool imColourPicker(const String& /*label*/, glm::vec4& /*colour*/, const platform::ImGUIColour& /*colour_type*/, const platform::ImGUITextAlign& /*text_align = platform::ImGUITextAlign::kLeft*/) override { return false; };
      inline virtual bool imTextEdit(const String& /*label*/, String& /*text*/) override { return false; };
      inline virtual bool imFloat1(const String& /*label*/, float& /*val*/) override { return false; };
      inline virtual bool imFloat2(const String& /*label*/, glm::vec2& /*val*/) override { return false; };
      inline virtual bool imFloat3(const String& /*label*/, glm::vec3& /*val*/) override { return false; };
      inline virtual bool imFloat4(const String& /*label*/, glm::vec4& /*val*/) override { return false; };
      inline virtual void imImage(const asset::VioletTextureHandle& /*texture*/, const glm::vec2& /*size*/) override {};

      inline virtual bool imMainMenuBarBegin(const uint8_t& /*menu_items*/) override { return false; };
      inline virtual void imMainMenuBarEnd() override {};
      inline virtual bool imMenuBegin(const String& /*label*/, const glm::vec2& /*size*/) override { return false; };
      inline virtual void imMenuEnd() override {};
    };
  }
}
