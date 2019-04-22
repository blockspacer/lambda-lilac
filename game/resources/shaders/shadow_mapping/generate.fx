[NO_DIRECTIONAL|VSM_DIRECTIONAL|ESM_DIRECTIONAL|NO_SPOT|VSM_SPOT|ESM_SPOT|NO_POINT|VSM_POINT|ESM_POINT]
#include "../common.fxh"

// Get the light type.
#if TYPE == NO_DIRECTIONAL || TYPE == VSM_DIRECTIONAL || TYPE == ESM_DIRECTIONAL
#define LIGHT_DIRECTIONAL 1
#elif TYPE == NO_SPOT || TYPE == VSM_SPOT || TYPE == ESM_SPOT
#define LIGHT_SPOT 1
#elif TYPE == NO_POINT || TYPE == VSM_POINT || TYPE == ESM_POINT
#define LIGHT_POINT 1
#endif

// Get the shadow type.
#if TYPE == VSM_DIRECTIONAL || TYPE == VSM_SPOT || TYPE == VSM_POINT
#define VSM 1
#define ESM 0
#define NO_SHADOW 0
#elif TYPE == ESM_DIRECTIONAL || TYPE == ESM_SPOT || TYPE == ESM_POINT
#define VSM 0
#define ESM 1
#define NO_SHADOW 0
#else
#define VSM 0
#define ESM 0
#define NO_SHADOW 1
#endif

struct VSInput
{
  float3 position : Positions;
  float2 tex      : TexCoords;
};

struct VSOutput
{
  float4 position  : SV_POSITION0;
  float4 hPosition : HPOSITION;
  float2 tex       : TEX_COORD;
};

VSOutput VS(VSInput vIn, uint instanceID : SV_InstanceID)
{
  VSOutput vOut;
  vOut.hPosition   = mul(model_matrix[instanceID], float4(vIn.position, 1.0f));
  vOut.position    = mul(light_view_projection_matrix, vOut.hPosition);
  vOut.tex         = vIn.tex;
  return vOut;
}

Make_Texture2D(tex_albedo, 0);

// TODO (Hilze): Support alpha.
float4 PS(VSOutput pIn) : SV_Target0
{
#if NO_SHADOW
  discard;
#endif

  if (tex_albedo.Sample(SamLinearWarp, pIn.tex).a < 0.5f)
    discard;

#if LIGHT_POINT || LIGHT_SPOT
  float3 position = pIn.hPosition.xyz;
  float depth = length(position - light_position);
#elif LIGHT_DIRECTIONAL
  float depth = pIn.position.z * light_far;
#else
  float depth = 0.0f;
#endif

#if VSM
  float dx = ddx(depth);
  float dy = ddy(depth);
  float moment2 = depth * depth + 0.25f * (dx * dx + dy * dy);
  
  return float4(depth, moment2, 1.0f, 1.0f);
#else
  return float4(depth, 1.0f, 1.0f, 1.0f);
#endif
}