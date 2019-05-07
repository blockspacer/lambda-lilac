#include "common.fxh"
#include "tbn.fxh"

#define VIOLET_PARALLAX_MAPPING 0
#define NORMAL_MAPPING 1
#define VIOLET_GRID_ALBEDO 0
#define VIOLET_GRID_TEXTURED 1
#define VIOLET_DYNAMIC_GRID_SIZE 1
#define VIOLET_DISPLACEMENT 1

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
  float2 mr        : METALLIC_ROUGHNESS;
  float3 emissive  : EMISSIVENESS;
  float3 normal    : NORMAL;
#if NORMAL_MAPPING
  float3x3 tbn     : TBN;
#endif
#if VIOLET_GRID_ALBEDO
  float3 tangent   : TANGENT;
  float3 bitangent : BITANGENT;
#endif
};

VSOutput VS(VSInput vIn, uint instanceID : SV_InstanceID)
{
  VSOutput vOut;
  vOut.hPosition = mul(model_matrix[instanceID], float4(vIn.position, 1.0f));
  vOut.position  = mul(view_projection_matrix, vOut.hPosition);
  vOut.colour    = float4(1.0f, 1.0f, 1.0f, 1.0f);
  vOut.tex       = vIn.tex;
  vOut.mr        = metallic_roughness[instanceID].xy;
  vOut.emissive  = emissiveness[instanceID].xyz;

#if NORMAL_MAPPING
  float3 bitangent = cross(vIn.tangent, vIn.normal);
  float3 N = normalize(mul((float3x3)model_matrix[instanceID], vIn.normal));
  float3 B = normalize(mul((float3x3)model_matrix[instanceID], bitangent));
  float3 T = normalize(mul((float3x3)model_matrix[instanceID], vIn.tangent));
  vOut.tbn = float3x3(T, B, N);
#endif
  vOut.normal    = normalize(mul((float3x3)model_matrix[instanceID], vIn.normal));
#if VIOLET_GRID_ALBEDO
  vOut.tangent   = normalize(mul((float3x3)model_matrix[instanceID], vIn.tangent));
  vOut.bitangent = cross(vOut.tangent, vOut.normal);
#endif

  return vOut;
}

Make_Texture2D(tex_albedo,   0);
Make_Texture2D(tex_normal,   1);
Make_Texture2D(tex_dmra,     2); // Displacement - Metallicness - Roughness - Ambient Occlusion
Make_Texture2D(tex_emissive, 3);

struct PSOutput
{
  float4 albedo   : SV_Target0;
  float4 position : SV_Target1;
  float4 normal   : SV_Target2;
  float4 mra      : SV_Target3; // Metallic - Roughness - Ambient Occlusion.
  float4 emissive : SV_Target4;
};

#if VIOLET_GRID_ALBEDO && VIOLET_DYNAMIC_GRID_SIZE
static const float kDynamicDistance = 20.0f * 20.0f;
#endif

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

#if VIOLET_GRID_ALBEDO && VIOLET_DYNAMIC_GRID_SIZE
float lengthSqr(const float3 v)
{
  return v.x * v.x + v.y * v.y + v.z * v.z;
}
#endif

[earlydepthstencil]
PSOutput PS(VSOutput pIn)
{
#if VIOLET_PARALLAX_MAPPING
  float3 eye = mul(pIn.tbn, normalize(pIn.hPosition.xyz - camera_position));
  pIn.tex = parallaxMapping(pIn.tex, eye);
#endif

  PSOutput pOut;
  const float4 albedo = tex_albedo.Sample(SamLinearWarp, pIn.tex) * pIn.colour;
  if (albedo.a < 0.5f)
    return pOut;

  pOut.albedo = albedo;
  pOut.albedo.a = 1.0f;

#if VIOLET_GRID_ALBEDO
  const float3 hPosition = pIn.hPosition.xyz;
#if VIOLET_DYNAMIC_GRID_SIZE
  const float scale = PI * (((lengthSqr(hPosition - camera_position) < kDynamicDistance) ? 1.0f : 0.1f));
#else
  const float scale = PI;
#endif
  const float dotTan = dot(normalize(pIn.tangent),   hPosition * scale);
  const float dotBit = dot(normalize(pIn.bitangent), hPosition * scale);
#if VIOLET_GRID_TEXTURED
  pOut.albedo.rgb = (tex_albedo.Sample(SamLinearWarp, float2(dotTan, dotBit)) * pIn.colour).rgb;
#else
  const float el = sin(dotTan) * sin(dotBit);
  const float es = sin(dotTan * 10.0f) * sin(dotBit * 10.0f);
	pOut.albedo.rgb = lerp(lerp(0.4f, 0.5f, when_ge(es, 0.0f)), lerp(0.9f, 1.0f, when_ge(es, 0.0f)), when_ge(el, 0.0f));
#endif
#endif

#if NORMAL_MAPPING
  float3 N = normalize(tex_normal.Sample(SamLinearWarp, pIn.tex).rgb * 2.0f - 1.0f);
  N = mul(N, pIn.tbn);
#else
  float3 N = normalize(pIn.normal);
#endif

  const float4 dmra = tex_dmra.Sample(SamLinearWarp, pIn.tex);
  const float3 mra    = dmra.gba * float3(pIn.mr, 1.0f);
  pOut.position = float4(pIn.hPosition.xyz, 1.0f);
  pOut.normal   = float4(N * 0.5f + 0.5f, 1.0f);
  pOut.mra      = float4(mra, 1.0f);
  pOut.emissive = float4(tex_emissive.Sample(SamLinearWarp, pIn.tex).rgb * pIn.emissive, 1.0f);

#if VIOLET_PARALLAX_MAPPING && VIOLET_DISPLACEMENT
  pOut.position.xyz += pIn.normal * dmra.r * height_scale * 2.5f;
#endif

  return pOut;
}
