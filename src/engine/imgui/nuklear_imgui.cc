#include "nuklear_imgui.h"
#include <glm/gtc/matrix_transform.hpp>

#include "interfaces/iworld.h"
#include "assets/asset_manager.h"
#include <Windows.h>
#include <utils/file_system.h>
#include <fstream>
#include "input/keyboard.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#include <nuklear.h>

#ifndef NUKLEAR_MAX_VERTICES
#define NUKLEAR_MAX_VERTICES 512 * 1024
#endif
#ifndef NUKLEAR_MAX_INDICES 
#define NUKLEAR_MAX_INDICES  128 * 1024
#endif
#ifndef NUKLEAR_BUFFER_SIZE
#define NUKLEAR_BUFFER_SIZE  4 * 1024
#endif

nk_context* k_context_;

enum theme { 
  THEME_BLACK, 
  THEME_WHITE, 
  THEME_RED, 
  THEME_BLUE, 
  THEME_DARK, 
  THEME_CUSTOM 
};

    ///////////////////////////////////////////////////////////////////////////
void setStyle(struct nk_context* ctx, enum theme theme)
{
  struct nk_color table[NK_COLOR_COUNT];
  if (theme == THEME_WHITE) {
    table[NK_COLOR_TEXT] = nk_rgba(70, 70, 70, 255);
    table[NK_COLOR_WINDOW] = nk_rgba(175, 175, 175, 255);
    table[NK_COLOR_HEADER] = nk_rgba(175, 175, 175, 255);
    table[NK_COLOR_BORDER] = nk_rgba(0, 0, 0, 255);
    table[NK_COLOR_BUTTON] = nk_rgba(185, 185, 185, 255);
    table[NK_COLOR_BUTTON_HOVER] = nk_rgba(170, 170, 170, 255);
    table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(160, 160, 160, 255);
    table[NK_COLOR_TOGGLE] = nk_rgba(150, 150, 150, 255);
    table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(120, 120, 120, 255);
    table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(175, 175, 175, 255);
    table[NK_COLOR_SELECT] = nk_rgba(190, 190, 190, 255);
    table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(175, 175, 175, 255);
    table[NK_COLOR_SLIDER] = nk_rgba(190, 190, 190, 255);
    table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(80, 80, 80, 255);
    table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(70, 70, 70, 255);
    table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(60, 60, 60, 255);
    table[NK_COLOR_PROPERTY] = nk_rgba(175, 175, 175, 255);
    table[NK_COLOR_EDIT] = nk_rgba(150, 150, 150, 255);
    table[NK_COLOR_EDIT_CURSOR] = nk_rgba(0, 0, 0, 255);
    table[NK_COLOR_COMBO] = nk_rgba(175, 175, 175, 255);
    table[NK_COLOR_CHART] = nk_rgba(160, 160, 160, 255);
    table[NK_COLOR_CHART_COLOR] = nk_rgba(45, 45, 45, 255);
    table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 0, 0, 255);
    table[NK_COLOR_SCROLLBAR] = nk_rgba(180, 180, 180, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(140, 140, 140, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(150, 150, 150, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(160, 160, 160, 255);
    table[NK_COLOR_TAB_HEADER] = nk_rgba(180, 180, 180, 255);
    nk_style_from_table(ctx, table);
  }
  else if (theme == THEME_RED) {
    table[NK_COLOR_TEXT] = nk_rgba(190, 190, 190, 255);
    table[NK_COLOR_WINDOW] = nk_rgba(30, 33, 40, 215);
    table[NK_COLOR_HEADER] = nk_rgba(181, 45, 69, 220);
    table[NK_COLOR_BORDER] = nk_rgba(51, 55, 67, 255);
    table[NK_COLOR_BUTTON] = nk_rgba(181, 45, 69, 255);
    table[NK_COLOR_BUTTON_HOVER] = nk_rgba(190, 50, 70, 255);
    table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(195, 55, 75, 255);
    table[NK_COLOR_TOGGLE] = nk_rgba(51, 55, 67, 255);
    table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(45, 60, 60, 255);
    table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(181, 45, 69, 255);
    table[NK_COLOR_SELECT] = nk_rgba(51, 55, 67, 255);
    table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(181, 45, 69, 255);
    table[NK_COLOR_SLIDER] = nk_rgba(51, 55, 67, 255);
    table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(181, 45, 69, 255);
    table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(186, 50, 74, 255);
    table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(191, 55, 79, 255);
    table[NK_COLOR_PROPERTY] = nk_rgba(51, 55, 67, 255);
    table[NK_COLOR_EDIT] = nk_rgba(51, 55, 67, 225);
    table[NK_COLOR_EDIT_CURSOR] = nk_rgba(190, 190, 190, 255);
    table[NK_COLOR_COMBO] = nk_rgba(51, 55, 67, 255);
    table[NK_COLOR_CHART] = nk_rgba(51, 55, 67, 255);
    table[NK_COLOR_CHART_COLOR] = nk_rgba(170, 40, 60, 255);
    table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 0, 0, 255);
    table[NK_COLOR_SCROLLBAR] = nk_rgba(30, 33, 40, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(64, 84, 95, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(70, 90, 100, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(75, 95, 105, 255);
    table[NK_COLOR_TAB_HEADER] = nk_rgba(181, 45, 69, 220);
    nk_style_from_table(ctx, table);
  }
  else if (theme == THEME_BLUE) {
    table[NK_COLOR_TEXT] = nk_rgba(20, 20, 20, 255);
    table[NK_COLOR_WINDOW] = nk_rgba(202, 212, 214, 215);
    table[NK_COLOR_HEADER] = nk_rgba(137, 182, 224, 220);
    table[NK_COLOR_BORDER] = nk_rgba(140, 159, 173, 255);
    table[NK_COLOR_BUTTON] = nk_rgba(137, 182, 224, 255);
    table[NK_COLOR_BUTTON_HOVER] = nk_rgba(142, 187, 229, 255);
    table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(147, 192, 234, 255);
    table[NK_COLOR_TOGGLE] = nk_rgba(177, 210, 210, 255);
    table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(182, 215, 215, 255);
    table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(137, 182, 224, 255);
    table[NK_COLOR_SELECT] = nk_rgba(177, 210, 210, 255);
    table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(137, 182, 224, 255);
    table[NK_COLOR_SLIDER] = nk_rgba(177, 210, 210, 255);
    table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(137, 182, 224, 245);
    table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(142, 188, 229, 255);
    table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(147, 193, 234, 255);
    table[NK_COLOR_PROPERTY] = nk_rgba(210, 210, 210, 255);
    table[NK_COLOR_EDIT] = nk_rgba(210, 210, 210, 225);
    table[NK_COLOR_EDIT_CURSOR] = nk_rgba(20, 20, 20, 255);
    table[NK_COLOR_COMBO] = nk_rgba(210, 210, 210, 255);
    table[NK_COLOR_CHART] = nk_rgba(210, 210, 210, 255);
    table[NK_COLOR_CHART_COLOR] = nk_rgba(137, 182, 224, 255);
    table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 0, 0, 255);
    table[NK_COLOR_SCROLLBAR] = nk_rgba(190, 200, 200, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(64, 84, 95, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(70, 90, 100, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(75, 95, 105, 255);
    table[NK_COLOR_TAB_HEADER] = nk_rgba(156, 193, 220, 255);
    nk_style_from_table(ctx, table);
  }
  else if (theme == THEME_DARK) {
    table[NK_COLOR_TEXT] = nk_rgba(210, 210, 210, 255);
    table[NK_COLOR_WINDOW] = nk_rgba(57, 67, 71, 215);
    table[NK_COLOR_HEADER] = nk_rgba(51, 51, 56, 220);
    table[NK_COLOR_BORDER] = nk_rgba(46, 46, 46, 255);
    table[NK_COLOR_BUTTON] = nk_rgba(48, 83, 111, 255);
    table[NK_COLOR_BUTTON_HOVER] = nk_rgba(58, 93, 121, 255);
    table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(63, 98, 126, 255);
    table[NK_COLOR_TOGGLE] = nk_rgba(50, 58, 61, 255);
    table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(45, 53, 56, 255);
    table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(48, 83, 111, 255);
    table[NK_COLOR_SELECT] = nk_rgba(57, 67, 61, 255);
    table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(48, 83, 111, 255);
    table[NK_COLOR_SLIDER] = nk_rgba(50, 58, 61, 255);
    table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(48, 83, 111, 245);
    table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(53, 88, 116, 255);
    table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(58, 93, 121, 255);
    table[NK_COLOR_PROPERTY] = nk_rgba(50, 58, 61, 255);
    table[NK_COLOR_EDIT] = nk_rgba(50, 58, 61, 225);
    table[NK_COLOR_EDIT_CURSOR] = nk_rgba(210, 210, 210, 255);
    table[NK_COLOR_COMBO] = nk_rgba(50, 58, 61, 255);
    table[NK_COLOR_CHART] = nk_rgba(50, 58, 61, 255);
    table[NK_COLOR_CHART_COLOR] = nk_rgba(48, 83, 111, 255);
    table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 0, 0, 255);
    table[NK_COLOR_SCROLLBAR] = nk_rgba(50, 58, 61, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(48, 83, 111, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(53, 88, 116, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(58, 93, 121, 255);
    table[NK_COLOR_TAB_HEADER] = nk_rgba(48, 83, 111, 255);
    nk_style_from_table(ctx, table);
  }
  else if (theme == THEME_CUSTOM)
  {
    ctx->style.window.background = nk_rgba(0, 0, 0, 0);
    ctx->style.window.fixed_background = 
      nk_style_item_color(nk_rgba(0, 0, 0, 0));
    ctx->style.window.border_color = nk_rgb(255, 165, 0);
    ctx->style.window.combo_border_color = nk_rgb(255, 165, 0);
    ctx->style.window.contextual_border_color = nk_rgb(255, 165, 0);
    ctx->style.window.menu_border_color = nk_rgb(255, 165, 0);
    ctx->style.window.group_border_color = nk_rgb(255, 165, 0);
    ctx->style.window.tooltip_border_color = nk_rgb(255, 165, 0);
    ctx->style.window.scrollbar_size = nk_vec2(16, 16);
    ctx->style.window.border_color = nk_rgba(0, 0, 0, 0);
    ctx->style.window.border = 1;

    ctx->style.button.normal = nk_style_item_color(nk_rgba(0, 0, 0, 0));
    ctx->style.button.hover = nk_style_item_color(nk_rgb(255, 165, 0));
    ctx->style.button.active = nk_style_item_color(nk_rgb(220, 10, 0));
    ctx->style.button.border_color = nk_rgb(255, 165, 0);
    ctx->style.button.text_background = nk_rgb(0, 0, 0);
    ctx->style.button.text_normal = nk_rgb(255, 165, 0);
    ctx->style.button.text_hover = nk_rgb(28, 48, 62);
    ctx->style.button.text_active = nk_rgb(28, 48, 62);
  }
  else {
    nk_style_default(ctx);
  }
}

namespace lambda
{
  namespace imgui
  {
    ///////////////////////////////////////////////////////////////////////////
    size_t NuklearImGUI::k_instance_count_ = 0u;

    ///////////////////////////////////////////////////////////////////////////
    void* nuklearAlloc(nk_handle unused, void* old, nk_size size)
    {
      NK_UNUSED(unused);
      NK_UNUSED(old);
      return foundation::Memory::allocate(size);
    }

    ///////////////////////////////////////////////////////////////////////////
    void nuklearFree(nk_handle unused, void* ptr)
    {
      NK_UNUSED(unused);
      if (ptr != nullptr)
      {
        foundation::Memory::deallocate(ptr);
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    void NuklearImGUI::initialize(world::IWorld* world)
    {
      world_ = world;

      k_instance_count_++;

      context_   = foundation::Memory::construct<nk_context>();
      allocator_ = foundation::Memory::construct<nk_allocator>();
      atlas_     = foundation::Memory::construct<nk_font_atlas>();
      commands_  = foundation::Memory::construct<nk_buffer>();
      null_      = foundation::Memory::construct<nk_null>();
      k_context_ = context_;

      max_vertex_count_ = NUKLEAR_MAX_VERTICES / sizeof(platform::ImGUIVertex);
      max_index_count_  = NUKLEAR_MAX_INDICES / sizeof(uint16_t);
      data_vertices_    = (platform::ImGUIVertex*)foundation::Memory::allocate(
        NUKLEAR_MAX_VERTICES
      );
      data_indices_     = (uint16_t*)foundation::Memory::allocate(
        NUKLEAR_MAX_INDICES
      );

      command_list_[0].mesh = command_list_[1].mesh = 
        asset::AssetManager::getInstance().createAsset(
          "__nuklear_mesh" + toString(k_instance_count_) + "___",
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

      command_list_[0].shader = command_list_[1].shader = 
        asset::AssetManager::getInstance().createAsset(
          "__nuklear_shader" + toString(k_instance_count_) + "__",
          foundation::Memory::constructShared<asset::Shader>(Name("imgui"), bytecode_vec)
      );

      // Initialize.
      allocator_->alloc = nuklearAlloc;
      allocator_->free  = nuklearFree;
      nk_init(context_, allocator_, nullptr);

      nk_buffer_init(commands_, allocator_, NUKLEAR_BUFFER_SIZE);

      white_texture_ = asset::TextureManager::getInstance()->create(
        Name("__nuklear_white" + toString(k_instance_count_) + "__"),
        1u, 1u, 1u, TextureFormat::kR8G8B8A8, 0u, Vector<unsigned char>{
        255u, 
        255u, 
        255u, 
        255u 
      }
      );
      null_->texture = nk_handle_ptr(&white_texture_);
    }

    ///////////////////////////////////////////////////////////////////////////
    void NuklearImGUI::update(const double& delta_time)
    {

      //static bool show_menu = true;
      //static bool show_app_about = true;
      //struct nk_rect bounds = nk_rect(0.0f, 0.0f, 1000.0f, 30.0f);
      //nk_begin(context_, "menu_bar", bounds, NK_WINDOW_NO_SCROLLBAR | 
      //  NK_WINDOW_BACKGROUND);
      //
      //enum menu_states { MENU_DEFAULT, MENU_WINDOWS };
      //static nk_size mprog = 60;
      //static int mslider = 10;
      //static int mcheck = nk_true;
      //nk_menubar_begin(context_);

      ///* menu #1 */
      //nk_layout_row_begin(context_, NK_STATIC, 25, 5);
      //nk_layout_row_push(context_, 45);
      //if (nk_menu_begin_label(context_, "MENU", NK_TEXT_LEFT, 
      //  nk_vec2(120, 200)))
      //{
      //  static size_t prog = 40;
      //  static int slider = 10;
      //  static int check = nk_true;
      //  nk_layout_row_dynamic(context_, 25, 1);
      //  if (nk_menu_item_label(context_, "Hide", NK_TEXT_LEFT))
      //    show_menu = nk_false;
      //  if (nk_menu_item_label(context_, "About", NK_TEXT_LEFT))
      //    show_app_about = nk_true;
      //  nk_progress(context_,&prog, 100, NK_MODIFIABLE);
      //  nk_slider_int(context_, 0,&slider, 16, 1);
      //  nk_checkbox_label(context_, "check",&check);
      //  nk_menu_end(context_);
      //}
      ///* menu #2 */
      //nk_layout_row_push(context_, 60);
      //if (nk_menu_begin_label(context_, "ADVANCED", NK_TEXT_LEFT, 
      //  nk_vec2(200, 600)))
      //{
      //  enum menu_state { MENU_NONE, MENU_FILE, MENU_EDIT, MENU_VIEW,
      //  MENU_CHART };
      //  static enum menu_state menu_state = MENU_NONE;
      //  enum nk_collapse_states state;

      //  state = (menu_state == MENU_FILE) ? NK_MAXIMIZED : NK_MINIMIZED;
      //  if (nk_tree_state_push(context_, NK_TREE_TAB, "FILE",&state)) {
      //    menu_state = MENU_FILE;
      //    nk_menu_item_label(context_, "New", NK_TEXT_LEFT);
      //    nk_menu_item_label(context_, "Open", NK_TEXT_LEFT);
      //    nk_menu_item_label(context_, "Save", NK_TEXT_LEFT);
      //    nk_menu_item_label(context_, "Close", NK_TEXT_LEFT);
      //    nk_menu_item_label(context_, "Exit", NK_TEXT_LEFT);
      //    nk_tree_pop(context_);
      //  }
      //  else menu_state = (menu_state == MENU_FILE) ? MENU_NONE : menu_state;

      //  state = (menu_state == MENU_EDIT) ? NK_MAXIMIZED : NK_MINIMIZED;
      //  if (nk_tree_state_push(context_, NK_TREE_TAB, "EDIT",&state)) {
      //    menu_state = MENU_EDIT;
      //    nk_menu_item_label(context_, "Copy", NK_TEXT_LEFT);
      //    nk_menu_item_label(context_, "Delete", NK_TEXT_LEFT);
      //    nk_menu_item_label(context_, "Cut", NK_TEXT_LEFT);
      //    nk_menu_item_label(context_, "Paste", NK_TEXT_LEFT);
      //    nk_tree_pop(context_);
      //  }
      //  else menu_state = (menu_state == MENU_EDIT) ? MENU_NONE : menu_state;

      //  state = (menu_state == MENU_VIEW) ? NK_MAXIMIZED : NK_MINIMIZED;
      //  if (nk_tree_state_push(context_, NK_TREE_TAB, "VIEW",&state)) {
      //    menu_state = MENU_VIEW;
      //    nk_menu_item_label(context_, "About", NK_TEXT_LEFT);
      //    nk_menu_item_label(context_, "Options", NK_TEXT_LEFT);
      //    nk_menu_item_label(context_, "Customize", NK_TEXT_LEFT);
      //    nk_tree_pop(context_);
      //  }
      //  else menu_state = (menu_state == MENU_VIEW) ? MENU_NONE : menu_state;

      //  state = (menu_state == MENU_CHART) ? NK_MAXIMIZED : NK_MINIMIZED;
      //  if (nk_tree_state_push(context_, NK_TREE_TAB, "CHART",&state)) {
      //    size_t i = 0;
      //    const float values[] = { 26.0f,13.0f,30.0f,15.0f,25.0f,10.0f,
      //      20.0f,40.0f,12.0f,8.0f,22.0f,28.0f };
      //    menu_state = MENU_CHART;
      //    nk_layout_row_dynamic(context_, 150, 1);
      //    nk_chart_begin(context_, NK_CHART_COLUMN, NK_LEN(values), 0, 50);
      //    for (i = 0; i < NK_LEN(values); ++i)
      //      nk_chart_push(context_, values[i]);
      //    nk_chart_end(context_);
      //    nk_tree_pop(context_);
      //  }
      //  else menu_state = (menu_state == MENU_CHART) ? 
      //         MENU_NONE : menu_state;
      //  nk_menu_end(context_);
      //}
      ///* menu widgets */
      //nk_layout_row_push(context_, 70);
      //nk_progress(context_,&mprog, 100, NK_MODIFIABLE);
      //nk_slider_int(context_, 0,&mslider, 16, 1);
      //nk_checkbox_label(context_, "check",&mcheck);
      //nk_menubar_end(context_);

      //nk_end(context_);
    }

    ///////////////////////////////////////////////////////////////////////////
    void NuklearImGUI::inputStart()
    {
      nk_input_begin(context_);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    bool NuklearImGUI::inputHandleMessage(
      const platform::WindowMessage& message)
    {
      if (nk_item_is_any_active(context_) == 0)
      {
        if(message.type == platform::WindowMessageType::kMouseMove)
        {
          nk_input_motion(
            context_, 
            (int)message.data[0], 
            (int)message.data[1]
          );
        }
        return false;
      }

      static glm::ivec2 mouse_position;
      switch(message.type)
      {
      case platform::WindowMessageType::kMouseButton:
      {
        if (message.data[0] <= 3)
        {
          nk_input_button(
            context_, 
            (nk_buttons)message.data[0], 
            mouse_position.x,
            mouse_position.y, 
            message.data[1]
          );
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
        mouse_position.x = (int)message.data[0];
        mouse_position.y = (int)message.data[1];
        nk_input_motion(context_, mouse_position.x, mouse_position.y);
        break;
      }
      case platform::WindowMessageType::kMouseScroll:
      {
        nk_input_scroll(context_, nk_vec2(0, (float)message.data[0]));
        break;
      }
      case platform::WindowMessageType::kKeyboardButton:
      {
        io::KeyboardKeys key = (io::KeyboardKeys)message.data[0];
        int down = (int)message.data[1];
        switch (key)
        {
        case io::KeyboardKeys::kBackspace:
          nk_input_key(context_, NK_KEY_BACKSPACE, down);
          break;
        case io::KeyboardKeys::kReturn:
          nk_input_key(context_, NK_KEY_ENTER, down);
          break;
        case io::KeyboardKeys::kControl:
          nk_input_key(context_, NK_KEY_CTRL, down);
          break;
        case io::KeyboardKeys::kShift:
          nk_input_key(context_, NK_KEY_SHIFT, down);
          break;
        case io::KeyboardKeys::kTab:
          nk_input_key(context_, NK_KEY_TAB, down);
          break;
        case io::KeyboardKeys::kLeft:
          nk_input_key(context_, NK_KEY_LEFT, down);
          break;
        case io::KeyboardKeys::kRight:
          nk_input_key(context_, NK_KEY_RIGHT, down);
          break;
        case io::KeyboardKeys::kUp:
          nk_input_key(context_, NK_KEY_UP, down);
          break;
        case io::KeyboardKeys::kDown:
          nk_input_key(context_, NK_KEY_DOWN, down);
          break;
          // TODO (Hilze): Add delete support.
        }
        break;
      }
      case platform::WindowMessageType::kChar:
      {
        if (message.data[0] != (uint32_t)io::KeyboardKeys::kEnter && 
          message.data[0] != (uint32_t)io::KeyboardKeys::kBackspace)
        {
          nk_input_unicode(context_, (nk_rune)message.data[0]);
        }
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

    ///////////////////////////////////////////////////////////////////////////
    void NuklearImGUI::inputEnd()
    {
      nk_input_end(context_);
    }

    ///////////////////////////////////////////////////////////////////////////
    void NuklearImGUI::setFont(const String& file_path, float font_size)
    {
      // Setup the atlas.
      nk_font_atlas_init(atlas_, allocator_);
      nk_font_atlas_begin(atlas_);

      Vector<char> file = FileSystem::FileToVector(file_path);

      // Get the custom font.
      struct nk_font_config font_config   = nk_font_config(font_size);
      font_config.oversample_v            = 1;
      font_config.oversample_h            = 1;
      font_config.ttf_blob                = (void*)file.data();
      font_config.ttf_size                = file.size();
      font_config.ttf_data_owned_by_atlas = 1;

      atlas_->default_font = nk_font_atlas_add(atlas_,&font_config);

      // Bake the font to a texture.
      const void* font_image = nullptr;
      int font_width  = 0;
      int font_height = 0;
      font_image = nk_font_atlas_bake(
        atlas_, 
        &font_width, 
        &font_height, 
        NK_FONT_ATLAS_RGBA32
      );

      // Gather the data for the texture.
      glm::uvec2 size(font_width, font_height);
      Vector<unsigned char> data(size.x * size.y * 4u);
      memcpy(data.data(), font_image, data.size());

      // Store the font.
      default_font_ = asset::TextureManager::getInstance()->create(
        Name("__nuklear_font" + toString(k_instance_count_) + "__"),
        (uint32_t)size.x,
        (uint32_t)size.y, 
        1u, 
        TextureFormat::kR8G8B8A8, 
        0u, 
        data
      );

      // End the font creation process.
      nk_font_atlas_end(atlas_, nk_handle_ptr(&default_font_), null_);
      nk_style_set_font(context_,&atlas_->default_font->handle);
      //nk_font_atlas_cleanup(atlas_);
    }

    ///////////////////////////////////////////////////////////////////////////
    void NuklearImGUI::endFrame()
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void NuklearImGUI::startFrame()
    {
      textures_.clear();
    }

    ///////////////////////////////////////////////////////////////////////////
    glm::mat4 getProjectionMatrix(unsigned int width, unsigned int height)
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

    ///////////////////////////////////////////////////////////////////////////
    void NuklearImGUI::generateCommandList()
    {
      pli_ = cli_;
      cli_ = cli_ == 0 ? 1 : 0;

      // Create the layout.
      NK_STORAGE const nk_draw_vertex_layout_element vertex_layout[] = {
        { NK_VERTEX_POSITION, NK_FORMAT_FLOAT, 
          NK_OFFSETOF(platform::ImGUIVertex, position) },
        { NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, 
          NK_OFFSETOF(platform::ImGUIVertex, uv)       },
        { NK_VERTEX_COLOR, NK_FORMAT_R32G32B32A32_FLOAT, 
          NK_OFFSETOF(platform::ImGUIVertex, colour)   },
        { NK_VERTEX_LAYOUT_END }
      };

      // Setup the config.
      nk_convert_config config{};
      config.vertex_layout        = vertex_layout;
      config.vertex_size          = sizeof(platform::ImGUIVertex);
      config.vertex_alignment     = NK_ALIGNOF(platform::ImGUIVertex);
      config.global_alpha         = 1.0f;
      config.shape_AA             = NK_ANTI_ALIASING_ON;
      config.line_AA              = NK_ANTI_ALIASING_ON;
      config.circle_segment_count = 22u;
      config.curve_segment_count  = 22u;
      config.arc_segment_count    = 22u;
      config.null                 = *null_;

      // Get the latest vertex and index buffer.
      nk_buffer vertices, indices;
      nk_buffer_init_fixed(
        &vertices, 
        data_vertices_, 
        (nk_size)NUKLEAR_MAX_VERTICES
      );
      nk_buffer_init_fixed(
        &indices,  
        data_indices_,  
        (nk_size)NUKLEAR_MAX_INDICES
      );
      nk_convert(context_, commands_,&vertices,&indices,&config);
      
      // Prepare the ImGUI command list.
      command_list_[cli_].projection = getProjectionMatrix(
        world_->getWindow()->getSize().x, 
        world_->getWindow()->getSize().y
      );
      command_list_[cli_].commands.resize(0u);
      Vector<asset::SubMesh> sub_meshes;
      Vector<asset::VioletTextureHandle> textures;

      // Loop over all of the commands.
      size_t offset = 0;
      const nk_draw_command* command;
      nk_draw_foreach(command, context_, commands_)
      {
        if (command->elem_count == 0u)
        {
          continue;
        }

        platform::ImGUICommand imgui_command;
        imgui_command.scissor_rect = glm::vec4(
          command->clip_rect.x, command->clip_rect.y,
          command->clip_rect.w, command->clip_rect.h
        );
        command_list_[cli_].commands.push_back(imgui_command);

        textures.push_back(*(asset::VioletTextureHandle*)command->texture.ptr);
        
        asset::SubMesh sub_mesh;
        sub_mesh.offset.at(asset::MeshElements::kPositions) = 
          asset::SubMesh::Offset(
            0u, 
            max_vertex_count_,   
            sizeof(platform::ImGUIVertex)
        );

        sub_mesh.offset.at(asset::MeshElements::kIndices) = 
          asset::SubMesh::Offset(
            0u, 
            command->elem_count, 
            sizeof(uint16_t)
        );
        
        sub_mesh.io.tex_alb = (int)textures.size() - 1;
        sub_mesh.index_offset = offset;
        sub_mesh.io.double_sided = true;
        sub_meshes.push_back(sub_mesh);

        offset += command->elem_count;
      }

      nk_clear(context_);

      if (command_list_[cli_].commands.size() > 0)
      {
        // Set the new vertex, index buffers, sub meshes, and textures.
        command_list_[cli_].mesh->set(
          asset::MeshElements::kPositions, 
          asset::Mesh::Buffer(
            data_vertices_, 
            (uint32_t)max_vertex_count_, 
            sizeof(platform::ImGUIVertex)
          )
        );

        command_list_[cli_].mesh->set(
          asset::MeshElements::kIndices,   
          asset::Mesh::Buffer(
            data_indices_,  
            (uint32_t)max_index_count_,  
            sizeof(uint16_t)
          )
        );

        command_list_[cli_].mesh->setSubMeshes(sub_meshes);
        command_list_[cli_].mesh->setAttachedTextureCount(
          glm::uvec3(textures.size(), 0u, 0u)
        );
        
        command_list_[cli_].mesh->setAttachedTextures(textures);
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    platform::ImGUICommandList NuklearImGUI::getCommandList()
    {
      return command_list_[pli_];
    }

    ///////////////////////////////////////////////////////////////////////////
    void NuklearImGUI::deinitialize()
    {
      nk_buffer_free(commands_);
      nk_free(context_);

      command_list_[0].mesh->clear();
      command_list_[1].mesh->clear();

      foundation::Memory::deallocate(atlas_);
      foundation::Memory::deallocate(allocator_);
      foundation::Memory::deallocate(null_);
      foundation::Memory::deallocate(data_vertices_);
      foundation::Memory::deallocate(data_indices_);
    }

    ///////////////////////////////////////////////////////////////////////////
    nk_flags textAlign(const platform::ImGUITextAlign& text_align)
    {
      nk_flags align;
      switch (text_align)
      {
      case platform::ImGUITextAlign::kLeft:
        align = nk_text_align::NK_TEXT_ALIGN_LEFT;
        break;
      case platform::ImGUITextAlign::kCenter:
        align = nk_text_align::NK_TEXT_ALIGN_CENTERED;
        break;
      case platform::ImGUITextAlign::kRight:
        align = nk_text_align::NK_TEXT_ALIGN_RIGHT;
        break;
      }
      return align | nk_text_align::NK_TEXT_ALIGN_MIDDLE;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    nk_color toColour(const glm::vec4& colour)
    {
      nk_color c;
      c.r = (nk_byte)(colour.x * 255.0f);
      c.g = (nk_byte)(colour.y * 255.0f);
      c.b = (nk_byte)(colour.z * 255.0f);
      c.a = (nk_byte)(colour.w * 255.0f);
      return c;
    }

    ///////////////////////////////////////////////////////////////////////////
    bool NuklearImGUI::imBegin(
      const String& name, 
      bool& is_open, 
      const glm::vec2& position, 
      const glm::vec2& size, 
      const uint8_t& flags)
    {
      uint32_t flgs = 0u;
      if ((flags& platform::ImGUIFlags::kMovable) > 0) 
        flgs |= NK_WINDOW_MOVABLE;
      if ((flags& platform::ImGUIFlags::kScalable) > 0) 
        flgs |= NK_WINDOW_SCALABLE;
      if ((flags& platform::ImGUIFlags::kClosable) > 0) 
        flgs |= NK_WINDOW_CLOSABLE;
      if ((flags& platform::ImGUIFlags::kMinimizable) > 0) 
        flgs |= NK_WINDOW_MINIMIZABLE;
      if ((flags& platform::ImGUIFlags::kTitle) > 0) 
        flgs |= NK_WINDOW_TITLE;
      if ((flags& platform::ImGUIFlags::kNoInput) > 0) 
        flgs |= NK_WINDOW_NO_INPUT;
      if ((flags& platform::ImGUIFlags::kNoScrollbar) > 0) 
        flgs |= NK_WINDOW_NO_SCROLLBAR;
      if (is_open == false) flgs |= NK_WINDOW_MINIMIZED;

      struct nk_rect bounds = nk_rect(position.x, position.y, size.x, size.y);
      is_open = nk_begin(context_, name.c_str(), bounds, flgs) > 0 ? true : false;

      return is_open;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    void NuklearImGUI::imEnd()
    {
      nk_end(context_);
    }

    ///////////////////////////////////////////////////////////////////////////
    void NuklearImGUI::imText(
      const String& text, 
      const platform::ImGUITextAlign& text_align)
    {
      nk_layout_row_dynamic(context_, 20, 1);
      nk_text(
        context_, 
        text.c_str(), 
        (int)text.length(), 
        textAlign(text_align)
      );
    }

    ///////////////////////////////////////////////////////////////////////////
    void lambda::imgui::NuklearImGUI::imTextMultiLine(
      const String& text, 
      const platform::ImGUITextAlign& text_align)
    {
      for (const String& line : split(text, '\n'))
      {
        imText(line);
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    void NuklearImGUI::imTextColoured(
      const String& text, 
      const glm::vec4& colour, 
      const platform::ImGUITextAlign& text_align)
    {
      nk_layout_row_dynamic(context_, 20, 1);
      nk_text_colored(
        context_, 
        text.c_str(), 
        (int)text.length(),
        textAlign(text_align), 
        toColour(colour)
      );
    }

    ///////////////////////////////////////////////////////////////////////////
    void NuklearImGUI::imLabel(
      const String& label, 
      const platform::ImGUITextAlign& text_align)
    {
      nk_layout_row_dynamic(context_, 20, 1);
      nk_label(context_, label.c_str(), textAlign(text_align));
    }

    ///////////////////////////////////////////////////////////////////////////
    bool NuklearImGUI::imButton(const String& label, const glm::vec2& size)
    {
      nk_layout_row_static(context_, size.y, (int)size.x, 1);
      return nk_button_label(context_, label.c_str()) > 0 ? true : false;
    }

    ///////////////////////////////////////////////////////////////////////////
    bool lambda::imgui::NuklearImGUI::imButtonImage(
      const asset::VioletTextureHandle& texture, 
      const glm::vec2& size)
    {
      if (textures_.find(texture.get()) == textures_.end())
        textures_.insert(eastl::make_pair(texture.get(), texture));

      nk_layout_row_static(context_, size.y, (int)size.x, 1);
      bool ret = nk_button_image(
        context_, 
        nk_image_handle(
          nk_handle_ptr((void*)&textures_.at(texture.get()))
        )
      ) > 0 ? true : false;

      return ret;
    }

    ///////////////////////////////////////////////////////////////////////////
    bool NuklearImGUI::imColourPicker(
      const String& label, 
      glm::vec4& colour, 
      const platform::ImGUIColour& colour_type, 
      const platform::ImGUITextAlign& text_align)
    {
      nk_colorf nc;
      float hsva[4];
      nk_color cf;

      if (colour_type == platform::ImGUIColour::kRGB)
      {
        memcpy(&nc,&colour, sizeof(colour));
      }
      else
      {
        memcpy(hsva,&colour, sizeof(colour));
        nc = nk_hsva_colorfv(hsva);
      }
      cf = nk_rgb_cf(nc);

      nk_layout_row_dynamic(context_, 25, 2);
      nk_label(context_, label.c_str(), textAlign(text_align));
      if (nk_combo_begin_color(
        context_, 
        cf, 
        nk_vec2(nk_widget_width(context_), 400)
      ))
      {
        nk_layout_row_dynamic(context_, 120, 1);
        nc = nk_color_picker(context_, nc, NK_RGBA);
        nk_layout_row_dynamic(context_, 25, 1);
        if (colour_type == platform::ImGUIColour::kRGB)
        {
          nc.r = nk_propertyf(context_, "#R:", 0.0f, nc.r, 1.0f, 0.01f, 0.01f);
          nc.g = nk_propertyf(context_, "#G:", 0.0f, nc.g, 1.0f, 0.01f, 0.01f);
          nc.b = nk_propertyf(context_, "#B:", 0.0f, nc.b, 1.0f, 0.01f, 0.01f);
          nc.a = nk_propertyf(context_, "#A:", 0.0f, nc.a, 1.0f, 0.01f, 0.01f);
          memcpy(&colour,&nc, sizeof(colour));
        }
        else
        {
          nk_colorf_hsva_fv(hsva, nc);
          hsva[0] = nk_propertyf(context_, "#H:", 0.0f, 
            hsva[0], 1.0f, 0.01f, 0.01f);
          hsva[1] = nk_propertyf(context_, "#S:", 0.0f, 
            hsva[1], 1.0f, 0.01f, 0.01f);
          hsva[2] = nk_propertyf(context_, "#V:", 0.0f, 
            hsva[2], 1.0f, 0.01f, 0.01f);
          hsva[3] = nk_propertyf(context_, "#A:", 0.0f, 
            hsva[3], 1.0f, 0.01f, 0.01f);
          memcpy(&colour, hsva, sizeof(colour));
        }
        nk_combo_end(context_);

        return true;
      }
      else
      {
        return false;
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    bool NuklearImGUI::imTextEdit(const String& label, String& text)
    {
      static const int k_buffer_size = 255;
      static char k_buffer[k_buffer_size];
      
      // Update buffer.
      int buffer_length = (int)text.size();
      memcpy(k_buffer, text.data(), buffer_length);
      
      // Text edit.
      nk_layout_row_dynamic(context_, 20, 2);
      nk_label(
        context_, 
        label.c_str(), 
        textAlign(platform::ImGUITextAlign::kLeft)
      );
      
      nk_edit_string(
        context_, 
        NK_EDIT_SIMPLE, 
        k_buffer, 
        &buffer_length, 
        k_buffer_size, 
        nk_filter_ascii
      ) == 1 ? true : false;

      // Update text.
      String str(buffer_length, '\0');
      memcpy((void*)str.data(), k_buffer, buffer_length);
      if (str != text)
      {
        text = str;
        return true;
      }
      else
      {
        return false;
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    bool lambda::imgui::NuklearImGUI::imFloat1(const String& label, float& val)
    {
      float v = val;
      nk_layout_row_dynamic(context_, 20, 2);
      nk_label(
        context_, 
        label.c_str(), 
        textAlign(platform::ImGUITextAlign::kLeft)
      );
      nk_property_float(context_, "#", FLT_MIN, &val, FLT_MAX, 0.01f, 0.001f);
      return v != val;
    }

    ///////////////////////////////////////////////////////////////////////////
    bool lambda::imgui::NuklearImGUI::imFloat2(
      const String& label, 
      glm::vec2& val)
    {
      glm::vec2 v = val;
      nk_layout_row_dynamic(context_, 20, 3);
      nk_label(
        context_, 
        label.c_str(), 
        textAlign(platform::ImGUITextAlign::kLeft)
      );
      
      nk_property_float(context_, "#", FLT_MIN,&val.x, FLT_MAX, 0.01f, 0.001f);
      nk_property_float(context_, "#", FLT_MIN,&val.y, FLT_MAX, 0.01f, 0.001f);
      return v != val;
    }

    ///////////////////////////////////////////////////////////////////////////
    bool lambda::imgui::NuklearImGUI::imFloat3(
      const String& label, 
      glm::vec3& val)
    {
      glm::vec3 v = val;
      nk_layout_row_dynamic(context_, 20, 4);
      nk_label(
        context_, 
        label.c_str(), 
        textAlign(platform::ImGUITextAlign::kLeft)
      );
      
      nk_property_float(context_, "#", FLT_MIN,&val.x, FLT_MAX, 0.01f, 0.001f);
      nk_property_float(context_, "#", FLT_MIN,&val.y, FLT_MAX, 0.01f, 0.001f);
      nk_property_float(context_, "#", FLT_MIN,&val.z, FLT_MAX, 0.01f, 0.001f);
      return v != val;
    }

    ///////////////////////////////////////////////////////////////////////////
    bool lambda::imgui::NuklearImGUI::imFloat4(
      const String& label, 
      glm::vec4& val)
    {
      glm::vec4 v = val;
      nk_layout_row_dynamic(context_, 20, 5);
      nk_label(
        context_, 
        label.c_str(), 
        textAlign(platform::ImGUITextAlign::kLeft)
      );
      
      nk_property_float(context_, "#", FLT_MIN,&val.x, FLT_MAX, 0.01f, 0.001f);
      nk_property_float(context_, "#", FLT_MIN,&val.y, FLT_MAX, 0.01f, 0.001f);
      nk_property_float(context_, "#", FLT_MIN,&val.z, FLT_MAX, 0.01f, 0.001f);
      nk_property_float(context_, "#", FLT_MIN,&val.w, FLT_MAX, 0.01f, 0.001f);
      return v != val;
    }

    ///////////////////////////////////////////////////////////////////////////
    void lambda::imgui::NuklearImGUI::imImage(
      const asset::VioletTextureHandle& texture, 
      const glm::vec2& size)
    {
      if (textures_.find(texture.get()) == textures_.end())
        textures_.insert(eastl::make_pair(texture.get(), texture));

      struct nk_rect bounds;
      bounds.x = nk_widget_position(context_).x;
      bounds.y = nk_widget_position(context_).y;
      bounds.w = size.x;
      bounds.h = size.y;
      nk_layout_row_static(context_, bounds.h, (int)bounds.w, 1);
      nk_widget(&bounds, context_);

      struct nk_image image_handle = nk_image_handle(
        nk_handle_ptr((void*)&textures_.at(texture.get()))
      );

      nk_draw_image(
        nk_window_get_canvas(context_),
        bounds,
       &image_handle,
        nk_rgb(255u, 255u, 255u)
      );
    }

    ///////////////////////////////////////////////////////////////////////////
    bool lambda::imgui::NuklearImGUI::imMainMenuBarBegin(
      const uint8_t& menu_items)
    {
      in_menu_ = true;
      struct nk_rect bounds = 
        nk_rect(0.0f, 0.0f, (float)world_->getWindow()->getSize().x, 30.0f);
      nk_begin(
        context_, 
        "menu_bar", 
        bounds, 
        NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND
      );
      
      nk_menubar_begin(context_);
      return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    void lambda::imgui::NuklearImGUI::imMainMenuBarEnd()
    {
      in_menu_ = false;
      nk_menubar_end(context_);
      nk_end(context_);
    }

    ///////////////////////////////////////////////////////////////////////////
    bool lambda::imgui::NuklearImGUI::imMenuBegin(
      const String& label, 
      const glm::vec2& size)
    {
      nk_layout_row_begin(context_, NK_STATIC, 25.0f, 1);
      nk_layout_row_push(context_, size.x);
      
      bool ret = nk_menu_begin_label(
        context_, 
        label.c_str(), 
        NK_TEXT_LEFT, 
        nk_vec2(size.x, size.y)
      ) > 0 ? true : false;

      return ret;
    }

    ///////////////////////////////////////////////////////////////////////////
    void lambda::imgui::NuklearImGUI::imMenuEnd()
    {
      nk_menu_end(context_);
    }
  }
}