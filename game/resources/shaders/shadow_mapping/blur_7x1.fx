[NO_HORIZONTAL|VSM_HORIZONTAL|ESM_HORIZONTAL|NO_VERTICAL|VSM_VERTICAL|ESM_VERTICAL|NO_HORIZONTAL_CUBE|VSM_HORIZONTAL_CUBE|ESM_HORIZONTAL_CUBE|CUBE]
#include "../common.fxh"

#if TYPE == NO_HORIZONTAL || TYPE == VSM_HORIZONTAL || TYPE == ESM_HORIZONTAL
static const float2 BLUR_SCALE = float2(1.0f, 0.0f);
#elif TYPE == NO_VERTICAL || TYPE == VSM_VERTICAL || TYPE == ESM_VERTICAL
static const float2 BLUR_SCALE = float2(0.0f, 1.0f);
#else
static const float BLUR_SCALE = 0.0f;
#endif

#if TYPE == CUBE
#define IS_CUBE_OUTPUT 1
#else
#define IS_CUBE_OUTPUT 0
#endif

#if TYPE == NO_HORIZONTAL_CUBE || TYPE == VSM_HORIZONTAL_CUBE || TYPE == ESM_HORIZONTAL_CUBE
#define IS_CUBE_INPUT 1
#else
#define IS_CUBE_INPUT 0
#endif

#if TYPE == ESM_HORIZONTAL || TYPE == ESM_VERTICAL || ESM_HORIZONTAL_CUBE
#define ESM 1
#define VSM 0
#define NO_SHADOW 0
#elif TYPE == VSM_HORIZONTAL || TYPE == VSM_VERTICAL || VSM_HORIZONTAL_CUBE
#define ESM 0
#define VSM 1
#define NO_SHADOW 0
#else
#define ESM 0
#define VSM 0
#define NO_SHADOW 1
#endif

struct VSOutput
{
  float2 tex      : TEX_COORD;
  float4 position : SV_POSITION;
};

VSOutput VS(uint id : SV_VertexID)
{
  VSOutput vOut;
  vOut.tex      = float2((id << 1) & 2, id & 2);
  vOut.position = float4(vOut.tex * float2(2, -2) + float2(-1, 1), 0, 1);

  return vOut;
}

#if IS_CUBE_INPUT || IS_CUBE_OUTPUT
#define face user_data[0].x
#endif

#define inv_texture_size user_data[15].xy

#if IS_CUBE_INPUT
Make_TextureCube(tex_to_blur, 0);
#else
Make_Texture2D(tex_to_blur, 0);
#endif

static const float blur_table[4] = {
  1.0f / 64.0f,
  6.0f / 64.0f,
  15.0f / 64.0f,
  20.0f / 64.0f,
};

#if VSM
#define BLUR(c, bc, w) c.xy += bc.xy * w
#elif ESM
#define BLUR(c, bc, w) c.x += bc.x * w
#else
#define BLUR(c, bc, w) c += bc * w
#endif

#if IS_CUBE_INPUT
float3 MAKE_CUBE(float2 i, float2 mod)
{
  i = i * 2.0f - 1.0f;

  switch (face)
  {
  default: return float3( 0.0f,  0.0f,  0.0f);
  case 0:  return float3( 1.0f,   i.x,   i.y);
  case 1:  return float3(-1.0f,   i.x,   i.y);
  case 2:  return float3(  i.x,  1.0f,   i.y);
  case 3:  return float3(  i.x, -1.0f,   i.y);
  case 4:  return float3(  i.x,   i.y,  1.0f);
  case 5:  return float3(  i.x,   i.y, -1.0f);
  }
}
#else
#define MAKE_CUBE(v, mod) float2(v + mod)
#endif

float4 PS(VSOutput pIn) : SV_TARGET0
{
#if IS_CUBE_OUTPUT
  switch (face)
  {
    default: return tex_to_blur.Sample(SamLinearClamp, pIn.tex);
    case 0: return tex_to_blur.Sample(SamLinearClamp, 1.0f - pIn.tex.yx);
    case 1: return tex_to_blur.Sample(SamLinearClamp, float2(1.0f - pIn.tex.y, pIn.tex.x));
    case 2: return tex_to_blur.Sample(SamLinearClamp, pIn.tex.xy);
    case 3: return tex_to_blur.Sample(SamLinearClamp, float2(pIn.tex.x, 1.0f - pIn.tex.y));
    case 4: return tex_to_blur.Sample(SamLinearClamp, float2(pIn.tex.x, 1.0f - pIn.tex.y));
    case 5: return tex_to_blur.Sample(SamLinearClamp, 1.0f - pIn.tex.xy);
  }
#elif NO_SHADOW
  return 0.0f;
#else
  float2 colour = 0.0f;

  const float2 scale = BLUR_SCALE * inv_texture_size;

  BLUR(colour, tex_to_blur.Sample(SamLinearClamp, MAKE_CUBE(pIn.tex, - 3.0f * scale)), blur_table[0]);
  BLUR(colour, tex_to_blur.Sample(SamLinearClamp, MAKE_CUBE(pIn.tex, - 2.0f * scale)), blur_table[1]);
  BLUR(colour, tex_to_blur.Sample(SamLinearClamp, MAKE_CUBE(pIn.tex, - 1.0f * scale)), blur_table[2]);
  BLUR(colour, tex_to_blur.Sample(SamLinearClamp, MAKE_CUBE(pIn.tex, + 0.0f * scale)), blur_table[3]);
  BLUR(colour, tex_to_blur.Sample(SamLinearClamp, MAKE_CUBE(pIn.tex, + 1.0f * scale)), blur_table[2]);
  BLUR(colour, tex_to_blur.Sample(SamLinearClamp, MAKE_CUBE(pIn.tex, + 2.0f * scale)), blur_table[1]);
  BLUR(colour, tex_to_blur.Sample(SamLinearClamp, MAKE_CUBE(pIn.tex, + 3.0f * scale)), blur_table[0]);

  return float4(colour, 1.0f, 1.0f);
#endif
}