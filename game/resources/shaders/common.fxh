#ifndef __COMMON__
#define __COMMON__

////////////////////////////////////////////////////////////////////////////////////////////////////
#if VIOLET_SPIRV
#define Lilac_CBuffer_Offset 0
#define Lilac_Texture_Offset 100
#define Lilac_SamplerState_Offset 200
#define Make_CBuffer_Internal(name, slot) [[vk::binding(slot + Lilac_CBuffer_Offset)]] cbuffer name : register(b##slot)
#define Make_Texture2D(name, slot) [[vk::binding(slot + Lilac_Texture_Offset)]] Texture2D name : register(t##slot)
#define Make_Texture2DArray(name, slot) [[vk::binding(slot + Lilac_Texture_Offset)]] Texture2DArray name : register(t##slot)
#define Make_TextureCube(name, slot) [[vk::binding(slot + Lilac_Texture_Offset)]] TextureCube name : register(t##slot)
#define Make_SamplerState(name, slot) [[vk::binding(slot + Lilac_SamplerState_Offset)]] SamplerState name : register(s##slot)
#else
#define Make_CBuffer_Internal(name, slot) cbuffer name : register(b##slot)
#define Make_Texture2D(name, slot) Texture2D name : register(t##slot)
#define Make_Texture2DArray(name, slot) Texture2DArray name : register(t##slot)
#define Make_TextureCube(name, slot) TextureCube name : register(t##slot)
#define Make_SamplerState(name, slot) SamplerState name : register(s##slot)
#endif
#define Make_CBuffer(name, slot) Make_CBuffer_Internal(name, slot)

////////////////////////////////////////////////////////////////////////////////////////////////////
static const float PI  = 3.14159265359f;
static const float TAU = 6.28318530718f;

////////////////////////////////////////////////////////////////////////////////////////////////////
Make_SamplerState(SamPointClamp, 6);
Make_SamplerState(SamLinearClamp, 7);
Make_SamplerState(SamAnisotrophicClamp, 8);
Make_SamplerState(SamPointBorder, 9);
Make_SamplerState(SamLinearBorder, 10);
Make_SamplerState(SamAnisotrophicBorder, 11);
Make_SamplerState(SamPointWarp, 12);
Make_SamplerState(SamLinearWarp, 13);
Make_SamplerState(SamAnisotrophicWarp, 14);

#define cbUserIdx 0
#define cbPerFrameIdx 1
#define cbPerCameraIdx 2
#define cbPerMeshIdx 3
#define cbPerTextureIdx 4
#define cbPerLightIdx 5
#define cbDynamicResolutionIdx 6

Make_CBuffer(cbPerFrame, cbPerFrameIdx)
{
  float2 screen_size;
  float delta_time;
  float total_time;
};

Make_CBuffer(cbPerCamera, cbPerCameraIdx)
{
  float4x4 view_matrix;
  float4x4 projection_matrix;
  float4x4 view_projection_matrix;
  float4x4 inverse_view_matrix;
  float4x4 inverse_projection_matrix;
  float4x4 inverse_view_projection_matrix;
  float3   camera_position;
  float    camera_far;
  float    camera_near;
};

Make_CBuffer(cbPerMesh, cbPerMeshIdx)
{
  float4x4 model_matrix;
  float2 metallic_roughness;
};

Make_CBuffer(cbPerTexture, cbPerTextureIdx)
{
  float2 inv_texture_size;
};

Make_CBuffer(cbPerLight, cbPerLightIdx)
{
  float4x4 light_view_projection_matrix;
  float3 light_position;
  float  light_near;
  float3 light_direction;
  float  light_far;
  float3 light_colour;
  float  light_cut_off;
  float  light_outer_cut_off;
};

Make_CBuffer(cbDynamicResolution, cbDynamicResolutionIdx)
{
  float dynamic_resolution_scale;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
float4 Sample(Texture2D t, SamplerState s, float2 tex)
{
  return t.Sample(s, tex * dynamic_resolution_scale);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static const float2 invAtan = float2(0.1591, 0.3183);
float2 SampleSphericalMap(float3 v)
{
  float2 uv = float2(atan2(v.z, v.x), -asin(v.y));
  return uv * invAtan + 0.5f;
}
float3 SampleSphericalMap(float2 uv)
{
  float2 thetaphi = ((uv * 2.0f) - 1.0f) * float2(3.1415926535897932384626433832795f, 1.5707963267948966192313216916398f);
  return float3(cos(thetaphi.y) * cos(thetaphi.x), -sin(thetaphi.y), cos(thetaphi.y) * sin(thetaphi.x));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------
float when_eq (float x, float y)    { return 1.0f - abs(sign(x - y)); }
float when_neq(float x, float y)    { return abs(sign(x - y));        }
float when_gt (float x, float y)    { return max(sign(x - y), 0.0f);  }
float when_lt (float x, float y)    { return max(sign(y - x), 0.0f);  }
float when_ge (float x, float y)    { return 1.0f - when_lt(x, y);    }
float when_le (float x, float y)    { return 1.0f - when_gt(x, y);    }
float and(float a, float b)         { return a * b;                   }
float or (float a, float b)         { return min(a + b, 1.0f);        }
float xor(float a, float b)         { return (a + b) % 2.0f;          }
float not(float a)                  { return 1.0f - a;                }
//---------------------------------------------------------------------
float2 when_eq (float2 x, float2 y) { return 1.0f - abs(sign(x - y)); }
float2 when_neq(float2 x, float2 y) { return abs(sign(x - y));        }
float2 when_gt (float2 x, float2 y) { return max(sign(x - y), 0.0f);  }
float2 when_lt (float2 x, float2 y) { return max(sign(y - x), 0.0f);  }
float2 when_ge (float2 x, float2 y) { return 1.0f - when_lt(x, y);    }
float2 when_le (float2 x, float2 y) { return 1.0f - when_gt(x, y);    }
float2 and(float2 a, float2 b)      { return a * b;                   }
float2 or (float2 a, float2 b)      { return min(a + b, 1.0f);        }
float2 xor(float2 a, float2 b)      { return (a + b) % 2.0f;          }
float2 not(float2 a)                { return 1.0f - a;                }
//---------------------------------------------------------------------
float3 when_eq (float3 x, float3 y) { return 1.0f - abs(sign(x - y)); }
float3 when_neq(float3 x, float3 y) { return abs(sign(x - y));        }
float3 when_gt (float3 x, float3 y) { return max(sign(x - y), 0.0f);  }
float3 when_lt (float3 x, float3 y) { return max(sign(y - x), 0.0f);  }
float3 when_ge (float3 x, float3 y) { return 1.0f - when_lt(x, y);    }
float3 when_le (float3 x, float3 y) { return 1.0f - when_gt(x, y);    }
float3 and(float3 a, float3 b)      { return a * b;                   }
float3 or (float3 a, float3 b)      { return min(a + b, 1.0f);        }
float3 xor(float3 a, float3 b)      { return (a + b) % 2.0f;          }
float3 not(float3 a)                { return 1.0f - a;                }
//---------------------------------------------------------------------
float4 when_eq (float4 x, float4 y) { return 1.0f - abs(sign(x - y)); }
float4 when_neq(float4 x, float4 y) { return abs(sign(x - y));        }
float4 when_gt (float4 x, float4 y) { return max(sign(x - y), 0.0f);  }
float4 when_lt (float4 x, float4 y) { return max(sign(y - x), 0.0f);  }
float4 when_ge (float4 x, float4 y) { return 1.0f - when_lt(x, y);    }
float4 when_le (float4 x, float4 y) { return 1.0f - when_gt(x, y);    }
float4 and(float4 a, float4 b)      { return a * b;                   }
float4 or (float4 a, float4 b)      { return min(a + b, 1.0f);        }
float4 xor(float4 a, float4 b)      { return (a + b) % 2.0f;          }
float4 not(float4 a)                { return 1.0f - a;                }
#endif // __COMMON__