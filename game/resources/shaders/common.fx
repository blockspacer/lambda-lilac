#ifndef __COMMON__
#define __COMMON__
////////////////////////////////////////////////////////////////////////////////////////////////////
SamplerState SamPointClamp   : register(s10);
SamplerState SamLinearClamp  : register(s11);
SamplerState SamPointBorder  : register(s12);
SamplerState SamLinearBorder : register(s13);
SamplerState SamPointWarp    : register(s14);
SamplerState SamLinearWarp   : register(s15);

////////////////////////////////////////////////////////////////////////////////////////////////////
cbuffer cbDynamicResolution
{
  float dynamic_resolution_scale = 0.5f;
};

float4 Sample(Texture2D t, SamplerState s, float2 tex)
{
  return t.Sample(s, tex * dynamic_resolution_scale);
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