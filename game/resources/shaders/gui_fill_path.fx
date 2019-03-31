#include "common.fx"

Make_CBuffer(Uniforms, 0)
{
 float4 State;
 float4x4 Transform;
 float4 Scalar4_0;
 float4 Scalar4_1;
 float4 Vector_0;
 float4 Vector_1;
 float4 Vector_2;
 float4 Vector_3;
 float4 Vector_4;
 float4 Vector_5;
 float4 Vector_6;
 float4 Vector_7;
 float ClipSize;
 float4x4 Clip_0;
 float4x4 Clip_1;
 float4x4 Clip_2;
 float4x4 Clip_3;
 float4x4 Clip_4;
 float4x4 Clip_5;
 float4x4 Clip_6;
 float4x4 Clip_7;
};

float Time() { return State[0]; }
float ScreenWidth() { return State[1]; }
float ScreenHeight() { return State[2]; }
float Scalar(int i) { if (i < 4) return Scalar4_0[i]; else return Scalar4_1[i - 4]; }

float4x4 GetClip(uint i)
{
  switch(i)
  {
    case 0: return Clip_0;
    case 1: return Clip_1;
    case 2: return Clip_2;
    case 3: return Clip_3;
    case 4: return Clip_4;
    case 5: return Clip_5;
    case 6: return Clip_6;
    case 7: return Clip_7;
    default: return 0.0f;
  }
}

float4 GetVector(uint i)
{
  switch(i)
  {
    case 0: return Vector_0;
    case 1: return Vector_1;
    case 2: return Vector_2;
    case 3: return Vector_3;
    case 4: return Vector_4;
    case 5: return Vector_5;
    case 6: return Vector_6;
    case 7: return Vector_7;
    default: return 0.0f;
  }
}

struct VS_OUTPUT
{
  float4 Position    : SV_POSITION;
  float4 Color       : COLOR0;
  float2 ObjectCoord : TEXCOORD0;
};

// VS
float2 ScreenToDeviceCoords(float2 screen_coord) {
  screen_coord *= 2.0 / float2(ScreenWidth(), -ScreenHeight());
  screen_coord += float2(-1.0, 1.0);
  return screen_coord;
}

VS_OUTPUT VS(float2 Position : inl_use_me_Positions,
            float4  Color    : inl_COLOR0,
            float2  ObjCoord : inl_TEXCOORD0)
{
  VS_OUTPUT output;
  float2 screen_coord = mul(Transform, float4(Position, 1.0, 1.0)).xy;
  output.Position = float4(ScreenToDeviceCoords(screen_coord), 1.0, 1.0);
  output.Color = Color;
  output.ObjectCoord = ObjCoord;
  return output;
}

// PS
Make_Texture2D(texture0, 0);
Make_SamplerState(sampler0, 0);

float sdRect(float2 p, float2 size) {
  float2 d = abs(p) - size;
  return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));
}

// The below function "sdEllipse" is MIT licensed with following text:
//
// The MIT License
// Copyright 2013 Inigo Quilez
// Permission is hereby granted, free of charge, to any person obtaining a 
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software
// is furnished to do so, subject to the following conditions: The above copyright
// notice and this permission notice shall be included in all copies or substantial
// portions of the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
// ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO
// EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

float sdEllipse(float2 p, float2 ab) {
  if (abs(ab.x - ab.y) < 0.1)
    return length(p) - ab.x;

  p = abs(p); if (p.x > p.y) { p = p.yx; ab = ab.yx; }

  float l = ab.y*ab.y - ab.x*ab.x;

  float m = ab.x*p.x / l;
  float n = ab.y*p.y / l;
  float m2 = m*m;
  float n2 = n*n;

  float c = (m2 + n2 - 1.0) / 3.0;
  float c3 = c*c*c;

  float q = c3 + m2*n2*2.0;
  float d = c3 + m2*n2;
  float g = m + m*n2;

  float co;

  if (d < 0.0) {
    float p = acos(q / c3) / 3.0;
    float s = cos(p);
    float t = sin(p)*sqrt(3.0);
    float rx = sqrt(-c*(s + t + 2.0) + m2);
    float ry = sqrt(-c*(s - t + 2.0) + m2);
    co = (ry + sign(l)*rx + abs(g) / (rx*ry) - m) / 2.0;
  } else {
    float h = 2.0*m*n*sqrt(d);
    float s = sign(q + h)*pow(abs(q + h), 1.0 / 3.0);
    float u = sign(q - h)*pow(abs(q - h), 1.0 / 3.0);
    float rx = -s - u - c*4.0 + 2.0*m2;
    float ry = (s - u)*sqrt(3.0);
    float rm = sqrt(rx*rx + ry*ry);
    float p = ry / sqrt(rm - rx);
    co = (p + 2.0*g / rm - m) / 2.0;
  }

  float si = sqrt(1.0 - co*co);

  float2 r = float2(ab.x*co, ab.y*si);

  return length(r - p) * sign(p.y - r.y);
}

// 1.0 = No softening, 0.1 = Max softening
#define SOFTEN_ELLIPSE 1.0

float sdRoundRect(float2 p, float2 size, float4 rx, float4 ry) {
  size *= 0.5;
  float2 corner;

  corner = float2(-size.x + rx.x, -size.y + ry.x);  // Top-Left
  float2 local = p - corner;
  if (dot(rx.x, ry.x) > 0.0 && p.x < corner.x && p.y <= corner.y)
    return sdEllipse(local, float2(rx.x, ry.x)) * SOFTEN_ELLIPSE;

  corner = float2(size.x - rx.y, -size.y + ry.y);   // Top-Right
  local = p - corner;
  if (dot(rx.y, ry.y) > 0.0 && p.x >= corner.x && p.y <= corner.y)
    return sdEllipse(local, float2(rx.y, ry.y)) * SOFTEN_ELLIPSE;

  corner = float2(size.x - rx.z, size.y - ry.z);  // Bottom-Right
  local = p - corner;
  if (dot(rx.z, ry.z) > 0.0 && p.x >= corner.x && p.y >= corner.y)
    return sdEllipse(local, float2(rx.z, ry.z)) * SOFTEN_ELLIPSE;

  corner = float2(-size.x + rx.w, size.y - ry.w); // Bottom-Left
  local = p - corner;
  if (dot(rx.w, ry.w) > 0.0 && p.x < corner.x && p.y > corner.y)
    return sdEllipse(local, float2(rx.w, ry.w)) * SOFTEN_ELLIPSE;

  return sdRect(p, size);
}

float2 transformAffine(float2 val, float2 a, float2 b, float2 c) {
  return val.x * a + val.y * b + c;
}

void Unpack(float4 x, out float4 a, out float4 b) {
  const float s = 65536.0;
  a = floor(x / s);
  b = floor(x - a * s);
}

#define AA_WIDTH 0.354

float antialias(in float d, in float width, in float median) {
  return smoothstep(median - width, median + width, d);
}

float4 GetCol(in float4x4 m, uint i) { return float4(m[0][i], m[1][i], m[2][i], m[3][i]); }

#define VISUALIZE_CLIP 0

void applyClip(VS_OUTPUT input, inout float4 outColor) {
  for (uint i = 0; i < (uint)ClipSize; i++) {
    float4x4 data = GetClip(i);
    float2 origin = GetCol(data, 0).xy;
    float2 size = GetCol(data, 0).zw;
    float4 radii_x, radii_y;
    Unpack(GetCol(data, 1), radii_x, radii_y);
    bool inverse = bool(GetCol(data, 3).z);

    float2 p = input.ObjectCoord;
    p = transformAffine(p, GetCol(data, 2).xy, GetCol(data, 2).zw, GetCol(data, 3).xy);
    p -= origin;
    float d_clip = sdRoundRect(p, size, radii_x, radii_y) * (inverse ? -1.0 : 1.0);

#if VISUALIZE_CLIP
    if (abs(d_clip) < 3.0)
      outColor = float4(0.9, 1.0, 0.0, 1.0);
#else
    float alpha = antialias(-d_clip, AA_WIDTH, -AA_WIDTH);
    outColor = float4(outColor.rgb * alpha, outColor.a * alpha);
#endif
  }
}

float4 PS(VS_OUTPUT input) : SV_Target
{
  float4 outColor = input.Color;
  applyClip(input, outColor);

  return outColor;
}