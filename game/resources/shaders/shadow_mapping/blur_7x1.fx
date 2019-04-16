[NO_HORIZONTAL|VSM_HORIZONTAL|ESM_HORIZONTAL|NO_VERTICAL|VSM_VERTICAL|ESM_VERTICAL|NO_HORIZONTAL_CUBE|VSM_HORIZONTAL_CUBE|ESM_HORIZONTAL_CUBE|NO_VERTICAL_CUBE|VSM_VERTICAL_CUBE|ESM_VERTICAL_CUBE]
#include "../common.fxh"

#if TYPE == NO_HORIZONTAL || TYPE == VSM_HORIZONTAL || TYPE == ESM_HORIZONTAL || TYPE == NO_HORIZONTAL_CUBE || TYPE == VSM_HORIZONTAL_CUBE || TYPE == ESM_HORIZONTAL_CUBE
static const float2 BLUR_SCALE = float2(1.0f, 0.0f);
#elif TYPE == NO_VERTICAL || TYPE == VSM_VERTICAL || TYPE == ESM_VERTICAL || TYPE == NO_VERTICAL_CUBE || TYPE == VSM_VERTICAL_CUBE || TYPE == ESM_VERTICAL_CUBE
static const float2 BLUR_SCALE = float2(0.0f, 1.0f);
#else
static const float BLUR_SCALE = 0.0f;
#endif

#if TYPE == VSM_HORIZONTAL_CUBE || TYPE == VSM_VERTICAL_CUBE || TYPE == ESM_HORIZONTAL_CUBE || TYPE == ESM_VERTICAL_CUBE
#define IS_CUBE 1
#else
#define IS_CUBE 0
#endif

#if TYPE == ESM_HORIZONTAL || TYPE == ESM_VERTICAL || TYPE == ESM_HORIZONTAL_CUBE || TYPE == ESM_VERTICAL_CUBE
#define ESM 1
#define VSM 0
#define NO_SHADOW 0
#elif TYPE == VSM_HORIZONTAL || TYPE == VSM_VERTICAL || TYPE == VSM_HORIZONTAL_CUBE || TYPE == VSM_VERTICAL_CUBE
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

Make_CBuffer(cbUser, cbUserIdx)
{
#if IS_CUBE
  float face;
#endif
};

#if IS_CUBE
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

#if IS_CUBE
float3 MAKE_CUBE(float2 i)
{
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
#define MAKE_CUBE(v) float2(v)
#endif

float4 PS(VSOutput pIn) : SV_TARGET0
{
#if NO_SHADOW
  return 0.0f;
#else
  float2 colour = 0.0f;

  const float2 scale = BLUR_SCALE * inv_texture_size;

  BLUR(colour, tex_to_blur.Sample(SamLinearClamp, MAKE_CUBE(pIn.tex - 3.0f * scale)), blur_table[0]);
  BLUR(colour, tex_to_blur.Sample(SamLinearClamp, MAKE_CUBE(pIn.tex - 2.0f * scale)), blur_table[1]);
  BLUR(colour, tex_to_blur.Sample(SamLinearClamp, MAKE_CUBE(pIn.tex - 1.0f * scale)), blur_table[2]);
  BLUR(colour, tex_to_blur.Sample(SamLinearClamp, MAKE_CUBE(pIn.tex + 0.0f * scale)), blur_table[3]);
  BLUR(colour, tex_to_blur.Sample(SamLinearClamp, MAKE_CUBE(pIn.tex + 1.0f * scale)), blur_table[2]);
  BLUR(colour, tex_to_blur.Sample(SamLinearClamp, MAKE_CUBE(pIn.tex + 2.0f * scale)), blur_table[1]);
  BLUR(colour, tex_to_blur.Sample(SamLinearClamp, MAKE_CUBE(pIn.tex + 3.0f * scale)), blur_table[0]);

  return float4(colour, 1.0f, 1.0f);
#endif
}