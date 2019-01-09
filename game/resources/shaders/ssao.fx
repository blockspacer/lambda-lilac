#include "resources/shaders/common.fx"

cbuffer cbPostProcess
{
  float4x4 view_matrix;
  float4x4 projection_matrix;

  float2 screen_size;

  float bias          = 0.025f;
  float sample_radius = 0.0025f;
};

float3 kernels[64] = {
  float3(-0.0315447f, -0.0232464f, 0.0238357f),
  float3(0.00785198f, -0.0673455f, 0.00563663f),
  float3(0.0185005f, 0.0448534f, 0.0197856f),
  float3(0.0312699f, -0.043974f, 0.00252555f),
  float3(0.00213241f, -0.00613377f, 0.00238856f),
  float3(-0.0226011f, 0.0511582f, 0.0806621f),
  float3(0.069743f, 0.0054254f, 0.00926419f),
  float3(-0.0140521f, 0.0336563f, 0.0761417f),
  float3(-0.0141209f, -0.0269007f, 0.021878f),
  float3(0.00995372f, 0.0192421f, 0.0371857f),
  float3(-0.0355547f, 0.0678244f, 0.0507865f),
  float3(0.030711f, -0.0866114f, 0.0640124f),
  float3(-0.031758f, -0.00888489f, 0.0535491f),
  float3(-0.0153732f, -0.0131963f, 0.0105311f),
  float3(-0.000193373f, 0.00570334f, 0.00652011f),
  float3(0.0401917f, 0.000449233f, 0.02564f),
  float3(0.013674f, -0.000169247f, 0.00373915f),
  float3(0.0421648f, -0.040139f, 0.0235752f),
  float3(-0.0328507f, 0.0609829f, 0.0390336f),
  float3(0.0827393f, -0.0844159f, 0.0714399f),
  float3(0.0851713f, -0.0973351f, 0.00206132f),
  float3(0.0980285f, 0.0344848f, 0.0979926f),
  float3(0.049063f, 0.0381704f, 0.0114537f),
  float3(-0.0822761f, 0.00733034f, 0.16322f),
  float3(-0.0219198f, 0.0851447f, 0.0335728f),
  float3(0.0104004f, -0.00894467f, 0.0400376f),
  float3(-0.00531921f, 0.00110069f, 0.00616288f),
  float3(0.0140045f, -0.000617863f, 0.194186f),
  float3(0.0248658f, 0.178001f, 0.142326f),
  float3(-0.0221349f, -0.0186879f, 0.0232489f),
  float3(-0.108036f, -0.0595405f, 0.000396159f),
  float3(-0.0505698f, 0.0224265f, 0.0501235f),
  float3(-0.117398f, -0.0671268f, 0.163161f),
  float3(-0.104875f, 0.0543914f, 0.057859f),
  float3(-0.000168996f, -0.229999f, 0.191581f),
  float3(0.00910703f, 0.0460269f, 0.0281041f),
  float3(0.211396f, -0.0399051f, 0.0310157f),
  float3(-0.281073f, -0.0130238f, 0.263091f),
  float3(-0.19428f, -0.155978f, 0.0829263f),
  float3(-0.255851f, 0.103669f, 0.213033f),
  float3(-0.0724531f, -0.00677189f, 0.0559312f),
  float3(-0.0479846f, 0.0408676f, 0.0216966f),
  float3(0.419113f, -0.0840001f, 0.0614351f),
  float3(-0.142124f, -0.117721f, 0.0123825f),
  float3(-0.100215f, -0.148081f, 0.15891f),
  float3(-0.175959f, -0.0557777f, 0.472969f),
  float3(0.166189f, -0.31206f, 0.372383f),
  float3(-0.148555f, 0.214538f, 0.140087f),
  float3(0.0558507f, 0.176665f, 0.210244f),
  float3(0.449903f, 0.132308f, 0.218952f),
  float3(0.206962f, 0.221446f, 0.540535f),
  float3(0.247703f, -0.148567f, 0.189487f),
  float3(-0.48472f, -0.105082f, 0.352459f),
  float3(-0.0621413f, 0.2381f, 0.450433f),
  float3(0.00817513f, 0.0056147f, 0.0100818f),
  float3(0.04268f, 0.00276263f, 0.0256127f),
  float3(0.289768f, -0.0150121f, 0.453261f),
  float3(-0.323969f, 0.449255f, 0.466839f),
  float3(0.0853368f, -0.700595f, 0.437232f),
  float3(-0.38395f, -0.0728528f, 0.213074f),
  float3(0.607353f, -0.0959567f, 0.372154f),
  float3(-0.0320814f, -0.068635f, 0.118854f),
  float3(0.000619743f, 0.000552596f, 1.0f),
  float3(0.284828f, 0.236879f, 0.166289f)
 };

struct VSOutput
{
  float4 position : SV_POSITION;
  float2 tex      : TEXCOORD;
  float2 noise_scale : NOISE_SCALE;
  float3 viewRay  : VIEW_RAY;
};

static const float gAspectRatio = 1.777777777777778f;
static const float gTanHalfFOV = tan(90.0f / 2.0f);

VSOutput VS(uint id: SV_VertexID)
{
  VSOutput vOut;
  vOut.tex      = float2((id << 1) & 2, id & 2);
  vOut.position = float4(vOut.tex * float2(2, -2) + float2(-1, 1), 0, 1);
  vOut.noise_scale = screen_size / float2(4.0f, 4.0f);
  vOut.viewRay.x = vOut.position.x * gAspectRatio * gTanHalfFOV;
  vOut.viewRay.y = vOut.position.y * gTanHalfFOV;
  vOut.viewRay.z = 1.0f;
  return vOut;
}

Texture2D tex_position  : register(t0);
Texture2D tex_normal    : register(t1);
Texture2D tex_random    : register(t2);
Texture2D tex_depth     : register(t3);

float4 PS(VSOutput pIn) : SV_Target0
{
  {
    float z = Sample(tex_depth, SamLinearClamp, pIn.tex).r;
    float3 origin = pIn.viewRay * z;

    float3 normal = normalize(Sample(tex_normal, SamLinearClamp, pIn.tex).xyz * 2.0f - 1.0f);
    normal = mul((float3x3)view_matrix, normal);

    float3 rvec = tex_random.Sample(SamLinearClamp, pIn.tex * 100.0f).rgb * 2.0f - 1.0f;
    rvec = float3(0.0f, 0.0f, 1.0f);
    float3 tangent = normalize(rvec - normal * dot(rvec, normal));
    float3 bitangent = cross(normal, tangent);
    float3x3 tbn = float3x3(tangent, bitangent, normal);

    return float4(tangent, 1.0f);

    float occlusion = 0.0;
    for (int i = 0; i < 64; ++i) {
    // get sample position:
      float3 sam = mul(tbn, kernels[i]);
      sam = sam * sample_radius + origin;

    // project sample position:
      float4 offset = float4(sam, 1.0);
      offset = mul(projection_matrix, offset);
      offset.xy /= offset.w;
      offset.xy = offset.xy * 0.25 + 0.5;
      offset.x = 1.0f - offset.x;

      // get sample depth:
      float sampleDepth = Sample(tex_depth, sam_linear, offset.xy).r;

      return abs(sampleDepth - z) * 100.0f;

      // range check & accumulate:
      float rangeCheck = abs(origin.z - sampleDepth) < sample_radius ? 1.0 : 0.0;
      occlusion += (sampleDepth <= sam.z ? 1.0 : 0.0) * rangeCheck;

      return occlusion;
    }

    occlusion = 1.0 - (occlusion / 64);
    return occlusion;
  }
}