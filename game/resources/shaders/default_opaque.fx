#include "common.fx"
#include "tbn.fx"

#define VIOLET_PARALLAX_MAPPING 0
#define NORMAL_MAPPING 0
#define VIOLET_GRID_ALBEDO 0
#define VIOLET_DYNAMIC_GRID_SIZE 1

struct VSInput
{
  float3 position : Positions;
  float3 normal   : Normals;
  float2 tex      : TexCoords;
#if NORMAL_MAPPING
  float3 tangent  : Tangents;
#endif
};

struct VSOutput
{
  float4 position  : SV_POSITION0;
  float4 hPosition : H_POSITION;
  float4 colour    : COLOUR;
  float2 tex       : TEX_COORD;
#if NORMAL_MAPPING
  float3x3 tbn     : TBN;
#endif
  float3 normal    : NORMAL;
};

Make_CBuffer(cbPerMesh, 0)
{
  float4x4 model_matrix;
  float4x4 view_projection_matrix;
}

VSOutput VS(VSInput vIn)
{
  VSOutput vOut;
  vOut.hPosition = mul(model_matrix, float4(vIn.position, 1.0f));
  vOut.position  = mul(view_projection_matrix, vOut.hPosition);
  vOut.colour    = float4(1.0f, 1.0f, 1.0f, 1.0f);
  vOut.tex       = vIn.tex;

#if NORMAL_MAPPING
  float3 bitangent = cross(vIn.tangent, vIn.normal);
  float3 N = normalize(mul((float3x3)model_matrix, vIn.normal));
  float3 B = normalize(mul((float3x3)model_matrix, bitangent));
  float3 T = normalize(mul((float3x3)model_matrix, vIn.tangent));
  vOut.tbn = float3x3(T, B, N);
#endif
  vOut.normal = normalize(mul((float3x3)model_matrix, vIn.normal));

  return vOut;
}

Make_Texture2D(tex_albedo, 0);
Make_Texture2D(tex_normal, 1);
Make_Texture2D(tex_dmra, 2); // Displacement - Metallicness - Roughness - Ambient Occlusion

struct PSOutput
{
  float4 albedo   : SV_Target0;
  float4 position : SV_Target1;
  float4 normal   : SV_Target2;
  float4 mra      : SV_Target3; // Metallic - Roughness - Ambient Occlusion.
};

Make_CBuffer(cbPerMesh, 1)
{
  float2 metallic_roughness;
  float3 camera_position;
}

#if VIOLET_PARALLAX_MAPPING
static const float height_scale = 0.03f;
static const int maxLayers = 8;
static const int minLayers = 4;

float2 parallaxMapping(float2 tex, float3 eye)
{
  // number of depth layers
  float numLayers = lerp(maxLayers, minLayers, abs(dot(float3(0.0, 0.0, 1.0), eye)));  
  // calculate the size of each layer
  float layerDepth = 1.0 / numLayers;
  // depth of current layer
  float currentLayerDepth = 0.0;
  // the amount to shift the texture coordinates per layer (from vector P)
  float2 P = eye.xy / eye.z * height_scale;
  float2 deltaTexCoords = P / numLayers;

  // get initial values
  float2 currentTexCoords    = tex;
  float currentDepthMapValue = (1.0f - tex_dmra.Sample(SamLinearWarp, currentTexCoords).r);
  
  float currentLayer = 0.0f;
  while (currentLayerDepth < currentDepthMapValue && currentLayer < maxLayers)
  {
    currentLayer++;
    // shift texture coordinates along direction of P
    currentTexCoords -= deltaTexCoords;
    // get depthmap value at current texture coordinates
    currentDepthMapValue = (1.0f - tex_dmra.Sample(SamLinearWarp, currentTexCoords).r);  
    // get depth of next layer
    currentLayerDepth += layerDepth;  
  }
  
  // get texture coordinates before collision (reverse operations)
  float2 prevTexCoords = currentTexCoords + deltaTexCoords;

  // get depth after and before collision for linear interpolation
  float afterDepth  = currentDepthMapValue - currentLayerDepth;
  float beforeDepth = (1.0f - tex_dmra.Sample(SamLinearWarp, prevTexCoords).r) - currentLayerDepth + layerDepth;

  // interpolation of texture coordinates
  float weight = afterDepth / (afterDepth - beforeDepth);
  float2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

  return finalTexCoords;
}
#endif

PSOutput PS(VSOutput pIn)
{
#if VIOLET_PARALLAX_MAPPING
  float3 eye = mul(pIn.tbn, normalize(pIn.hPosition.xyz - camera_position));
  pIn.tex = parallaxMapping(pIn.tex, eye);
#endif

  PSOutput pOut;
  pOut.albedo = tex_albedo.Sample(SamLinearWarp, pIn.tex) * pIn.colour;
  if (tex_albedo.Sample(SamLinearWarp, pIn.tex).a < 0.25f)
    discard;
  pOut.albedo.a = 1.0f;

#if VIOLET_GRID_ALBEDO
#if VIOLET_DYNAMIC_GRID_SIZE
  const float scale = PI * ((length(pIn.hPosition - camera_position) < 100.0f) ? (length(pIn.hPosition - camera_position) < 10.0f) ? 10.0f : 1.0f : 0.1f);
#else
  const float scale = PI;
#endif
  const float3 pl = pIn.hPosition * scale;
  const float3 ps = pIn.hPosition * (scale * 10.0f);

  float el = sin(pl.x) * sin(pl.y) * sin(pl.z);
  float es = sin(ps.x) * sin(ps.y) * sin(ps.z);

	pOut.albedo.rgb = lerp(lerp(0.4, 0.5, when_ge(es, 0.0f)), lerp(0.9, 1.0, when_ge(es, 0.0f)), when_ge(el, 0.0f));
#endif

#if NORMAL_MAPPING
  float3 N = normalize(tex_normal.Sample(SamLinearWarp, pIn.tex).rgb * 2.0f - 1.0f);
  N = mul(N, pIn.tbn);
#else
  float3 N = normalize(pIn.normal);
#endif

  float3 mra    = tex_dmra.Sample(SamLinearWarp, pIn.tex).gba * float3(metallic_roughness, 1.0f);
  pOut.position = float4(pIn.hPosition.xyz, 1.0f);
  pOut.normal   = float4(N * 0.5f + 0.5f, 1.0f);
  pOut.mra      = float4(mra, 1.0f);

  return pOut;
}
