#pragma once
#include "interfaces/iimgui.h"

struct ImGuiContext;

namespace lambda
{
  namespace imgui
  {
    class DearImGUI : public platform::IImGUI
    {
    public:
      virtual String name() const override { return "dear imgui"; };
      virtual void setWindow(foundation::SharedPointer<platform::IWindow> window) override;
      virtual void initialize() override;
      virtual void update(const double& delta_time) override;
      virtual void deinitialize() override;
      virtual void inputStart() override;
      virtual bool inputHandleMessage(const platform::WindowMessage& message) override;
      virtual void inputEnd() override;
      virtual void setFont(const String& file_path, float font_size) override;
      virtual void endFrame() override;
      virtual void startFrame() override;
      virtual void generateCommandList() override;
      inline virtual platform::ImGUICommandList getCommandList() override { return platform::ImGUICommandList(); };

      virtual bool imBegin(const String& name, bool& is_open, const glm::vec2& position, const glm::vec2& size, const uint8_t& flags = platform::IImGUI::kDefaultWindowFlags) override;
      virtual void imEnd() override;
      virtual void imText(const String& text, const platform::ImGUITextAlign& text_align = platform::ImGUITextAlign::kLeft) override;
      virtual void imTextMultiLine(const String& text, const platform::ImGUITextAlign& text_align = platform::ImGUITextAlign::kLeft) override;
      virtual void imTextColoured(const String& text, const glm::vec4& colour, const platform::ImGUITextAlign& text_align = platform::ImGUITextAlign::kLeft) override;
      virtual void imLabel(const String& label, const platform::ImGUITextAlign& text_align = platform::ImGUITextAlign::kLeft) override;
      virtual bool imButton(const String& label, const glm::vec2& size) override;
      virtual bool imButtonImage(const asset::VioletTextureHandle& texture, const glm::vec2& size) override;
      virtual bool imColourPicker(const String& label, glm::vec4& colour, const platform::ImGUIColour& colour_type, const platform::ImGUITextAlign& text_align = platform::ImGUITextAlign::kLeft) override;
      virtual bool imTextEdit(const String& label, String& text) override;
      virtual bool imFloat1(const String& label, float& val) override;
      virtual bool imFloat2(const String& label, glm::vec2& val) override;
      virtual bool imFloat3(const String& label, glm::vec3& val) override;
      virtual bool imFloat4(const String& label, glm::vec4& val) override;
      virtual void imImage(const asset::VioletTextureHandle& texture, const glm::vec2& size) override;

      virtual bool imMainMenuBarBegin(const uint8_t& menu_items) override;
      virtual void imMainMenuBarEnd() override;
      virtual bool imMenuBegin(const String& label, const glm::vec2& size) override;
      virtual void imMenuEnd() override;

    private:
      void pushId(const String& label);
      void pushId(const String& label, void* val);

    public:
      foundation::SharedPointer<platform::IWindow> window_;
      bool menu_bar_open_ = false;
      bool menu_open_ = false;

      ImGuiContext* context_;
      
      platform::ImGUICommandList command_list_;
      platform::ImGUIVertex* data_vertices_;
      size_t max_vertex_count_;
      uint16_t* data_indices_;
      size_t max_index_count_;
      Map<const void*, asset::VioletTextureHandle> textures_;

      asset::VioletTextureHandle default_font_;
      static size_t k_instance_count_;
    };
  }
}