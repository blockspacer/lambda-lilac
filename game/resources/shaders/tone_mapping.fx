#include "common.fxh"

struct VSOutput
{
  float4 position : SV_POSITION0;
  float2 tex      : TEX_COORD;
};

VSOutput VS(uint id: SV_VertexID)
{
  VSOutput vOut;
  vOut.tex = float2((id << 1) & 2, id & 2);
  vOut.position = float4(vOut.tex * float2(2, -2) + float2(-1, 1), 0, 1);
  return vOut;
}

Make_Texture2D(tex_albedo, 0);

float3 hejlToneMapping(float3 color)
{
  float3 x = max(float3(0.0f, 0.0f, 0.0f), color - float3(0.004f, 0.004f, 0.004f));
  return (x * ((6.2f * x) + float3(0.5f, 0.5f, 0.5f))) / max(x * ((6.2f * x) + float3(1.7f, 1.7f, 1.7f)) + float3(0.06f, 0.06f, 0.06f), float3(1e-8f, 1e-8f, 1e-8f));
}

float3 HUEtoRGB(in float H)
{
  float R = abs(H * 6 - 3) - 1;
  float G = 2 - abs(H * 6 - 2);
  float B = 2 - abs(H * 6 - 4);
  return saturate(float3(R, G, B));
}
float3 HSVtoRGB(in float3 HSV)
{
  float3 RGB = HUEtoRGB(HSV.x);
  return ((RGB - 1) * HSV.y + 1) * HSV.z;
}
static const float Epsilon = 1e-10;
float3 RGBtoHCV(in float3 RGB)
{
  // Based on work by Sam Hocevar and Emil Persson
  float4 P = (RGB.g < RGB.b) ? float4(RGB.bg, -1.0, 2.0 / 3.0) : float4(RGB.gb, 0.0, -1.0 / 3.0);
  float4 Q = (RGB.r < P.x) ? float4(P.xyw, RGB.r) : float4(RGB.r, P.yzx);
  float C = Q.x - min(Q.w, Q.y);
  float H = abs((Q.w - Q.y) / (6 * C + Epsilon) + Q.z);
  return float3(H, C, Q.x);
}
float3 RGBtoHSV(in float3 RGB)
{
  float3 HCV = RGBtoHCV(RGB);
  float S = HCV.y / (HCV.z + Epsilon);
  return float3(HCV.x, S, HCV.z);
}

float3 colourToneMapping(float3 albedo, float3 col_hsv, float min_diff = 0.1f, float smooth_edge = 0.01f)
{
  float3 alb_hsv = RGBtoHSV(albedo);
  float luminance = dot(albedo, float3(0.299f, 0.587f, 0.114f));

  float val = min(min(abs(alb_hsv.x - col_hsv.x), abs(alb_hsv.x - col_hsv.x + 1.0f)), abs(alb_hsv.x - col_hsv.x - 1.0f));
  val = 1.0f - clamp((val - min_diff) / min_diff, 0.0f, 1.0f);
  return lerp(luminance, albedo, val);
}

float3 pbrToneMapping(float3 albedo)
{
  return pow(albedo / (albedo + 1.0f), 1.0f / 2.2f); 
}


float4 PS(VSOutput pIn) : SV_TARGET0
{
  float4 raw_albedo = Sample(tex_albedo, SamLinearClamp, pIn.tex);
  float3 tone_mapped = hejlToneMapping(raw_albedo.rgb);

  //static const float3 hsv = RGBtoHSV(float3(1.0f, 0.0f, 0.0f));
  //tone_mapped = colourToneMapping(tone_mapped, hsv, 0.25f);
  
  return float4(tone_mapped, raw_albedo.a);
}
