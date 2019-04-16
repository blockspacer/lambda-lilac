[STRENGTH1|STRENGTH2|STRENGTH3|STRENGTH4|STRENGTH5]
#include "common.fxh"

// Make_CBuffer(cbUser, cbUserIdx)
// {
//   float bias          = 0.025f;
//   float sample_radius = 0.0025f;
// };
static const float bias          = 0.025f;
static const float sample_radius = 0.0025f;


#if TYPE == STRENGTH1 || TYPE == DEFAULT
static const float kStrength = 1.0f;
#elif TYPE == STRENGTH2
static const float kStrength = 2.0f;
#elif TYPE == STRENGTH3
static const float kStrength = 3.0f;
#elif TYPE == STRENGTH4
static const float kStrength = 4.0f;
#elif TYPE == STRENGTH5
static const float kStrength = 5.0f;
#endif

static const int kKernelCount = 64;
static const float3 kKernels[kKernelCount] = {
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
};

VSOutput VS(uint id: SV_VertexID)
{
  VSOutput vOut;
  vOut.tex      = float2((id << 1) & 2, id & 2);
  vOut.position = float4(vOut.tex * float2(2, -2) + float2(-1, 1), 0, 1);
  return vOut;
}

Make_Texture2D(tex_position, 0);
Make_Texture2D(tex_normal, 1);
Make_Texture2D(tex_random, 2);
Make_Texture2D(tex_depth, 3);

static const float kSurfaceEpsilon = 0.025f;
static const float kOcclusionRadius = 0.125f;
static const float kOcclusionFadeStart = 0.2f;
static const float kOcclusionFadeEnd = 1.0f;

static const float4x4 kProjTex = float4x4(
  0.5f, 0.0f, 0.0f, 0.0f,
  0.0f, -0.5f, 0.0f, 0.0f,
  0.0f, 0.0f, 1.0f, 0.0f,
  0.5f, 0.5f, 0.0f, 1.0f
);

// Determines how much the sample point q occludes the point p as a function
// of distZ.
float OcclusionFunction(float distZ)
{
	//
	// If depth(q) is "behind" depth(p), then q cannot occlude p.  Moreover, if 
	// depth(q) and depth(p) are sufficiently close, then we also assume q cannot
	// occlude p because q needs to be in front of p by Epsilon to occlude p.
	//
	// We use the following function to determine the occlusion.  
	// 
	//
	//       1.0     -------------\
	//               |           |  \
	//               |           |    \
	//               |           |      \ 
	//               |           |        \
	//               |           |          \
	//               |           |            \
	//  ------|------|-----------|-------------|---------|--> zv
	//        0     Eps          z0            z1        
	//
	
#if NEW_METHOD
	float occlusion = 0.0f;
  occlusion = 1.0f - saturate(clamp((distZ - kOcclusionRadius) / kOcclusionRadius, 0.0f, 1.0f));

	return occlusion;	

#else
	float occlusion = 0.0f;
	if(distZ > kSurfaceEpsilon)
	{
		float fadeLength = kOcclusionFadeEnd - kOcclusionFadeStart;
		
		// Linearly decrease occlusion from 1 to 0 as distZ goes 
		// from gOcclusionFadeStart to gOcclusionFadeEnd.	
		occlusion = saturate((kOcclusionFadeEnd - distZ) / fadeLength);
	}
	
	return occlusion;	
#endif
}

float4 PS(VSOutput pIn) : SV_Target0
{
  // p -- the point we are computing the ambient occlusion for.
	// n -- normal vector at p.
	// q -- a random offset from p.
	// r -- a potential occluder that might occlude p.

	// Get viewspace normal and z-coord of this pixel.  
  const float3 n = normalize(mul(view_matrix, float4(Sample(tex_normal, SamLinearClamp, pIn.tex).xyz * 2.0f - 1.0f, 0.0f)).xyz);

	//
	// Reconstruct full view space position (x,y,z).
	// Find t such that p = t*pin.PosV.
	// p.z = t*pin.PosV.z
	// t = p.z / pin.PosV.z
	//
	const float3 p = mul(view_matrix, float4(Sample(tex_position, SamLinearClamp, pIn.tex).xyz, 1.0f)).xyz;
	
  const float2 noise_scale = screen_size;
  
	// Extract random vector and map from [0,1] --> [-1, +1].
	const float3 randVec = normalize(2.0f * tex_random.Sample(SamLinearWarp, pIn.tex * noise_scale).rgb - 1.0f);

	float occlusionSum = 0.0f;
	
	// Sample neighboring points about p in the hemisphere oriented by n.
	for (int i = 0; i < kKernelCount; ++i)
	{
		// Are offset vectors are fixed and uniformly distributed (so that our offset vectors
		// do not clump in the same direction).  If we reflect them about a random vector
		// then we get a random uniform distribution of offset vectors.
		const float3 offset = normalize(reflect(kKernels[i].xyz, randVec));

		// Flip offset vector if it is behind the plane defined by (p, n).
		const float flip = sign(dot(offset, n));
		
		// Sample a point near p within the occlusion radius.
		const float3 q = p + flip * kOcclusionRadius * offset;

		// Project q and generate projective tex-coords.  
		float4 projQ = mul(mul(projection_matrix, float4(q, 1.0f)), kProjTex);
		projQ /= projQ.w;

		// Find the nearest depth value along the ray from the eye to q (this is not
		// the depth of q, as q is just an arbitrary point near p and might
		// occupy empty space).  To find the nearest depth we look it up in the depthmap.

		const float rz = Sample(tex_depth, SamLinearClamp, projQ.xy).r;
    const float3 r = mul(view_matrix, float4(Sample(tex_position, SamLinearClamp, projQ.xy).xyz, 1.0f)).xyz;

		//
		// Test whether r occludes p.
		//   * The product dot(n, normalize(r - p)) measures how much in front
		//     of the plane(p,n) the occluder point r is.  The more in front it is, the
		//     more occlusion weight we give it.  This also prevents self shadowing where 
		//     a point r on an angled plane (p,n) could give a false occlusion since they
		//     have different depth values with respect to the eye.
		//   * The weight of the occlusion is scaled based on how far the occluder is from
		//     the point we are computing the occlusion of.  If the occluder r is far away
		//     from p, then it does not occlude it.
		// 
		
		const float distZ = p.z - r.z;
		const float dp = max(dot(n, normalize(r - p)), 0.0f);
    const float len = (length(r - p));

    const float occlusion = dp * OcclusionFunction(len);

		occlusionSum += occlusion;
	}
	
	occlusionSum /= kKernelCount;
	
	const float access = 1.0f - occlusionSum;

	// Sharpen the contrast of the SSAO map to make the SSAO affect more dramatic.
	return saturate(pow(access, kStrength));
}