#include "common.fx"

Make_CBuffer(cbPostProcess, 0)
{
  float4x4 view_matrix;
  float4x4 projection_matrix;
  float4x4 view_projection_matrix;

  float2 screen_size;

  float bias          = 0.025f;
  float sample_radius = 0.0025f;
};

static const int kRandomSampleCount = 64;
static const float3 kRandomSamples[kRandomSampleCount] = {
  float3(0.1453456543f, 0.2303844597f, 0.7091501878f),
  float3(0.8513219330f, 0.6057660882f, 0.3832141836f),
  float3(0.0685194244f, 0.2995445308f, 0.4057671909f),
  float3(0.1959306770f, 0.6806641285f, 0.7981706996f),
  float3(0.9690964526f, 0.1412088144f, 0.0631454864f),
  float3(0.2554281206f, 0.2904958438f, 0.7914625842f),
  float3(0.5551063996f, 0.4556745595f, 0.3399498579f),
  float3(0.5143105487f, 0.6052691201f, 0.9824557943f),
  float3(0.0432867037f, 0.8120491611f, 0.3815219864f),
  float3(0.7031360021f, 0.0473274017f, 0.6748201271f),
  float3(0.6881253743f, 0.2582156747f, 0.3292751940f),
  float3(0.9524493203f, 0.5776999897f, 0.4781562778f),
  float3(0.1091544746f, 0.8329969425f, 0.2053135803f),
  float3(0.6001743676f, 0.7421152779f, 0.0018364749f),
  float3(0.5751294275f, 0.0080045361f, 0.9018720277f),
  float3(0.8371421602f, 0.1988119749f, 0.7144742714f),
  float3(0.0458528810f, 0.6539226643f, 0.2930603621f),
  float3(0.9900349413f, 0.7689775136f, 0.2973142027f),
  float3(0.1117706742f, 0.6528744025f, 0.5307050935f),
  float3(0.0827065244f, 0.1460313220f, 0.3662175005f),
  float3(0.8367719527f, 0.7392563299f, 0.5281000677f),
  float3(0.0390851936f, 0.9436695609f, 0.5459639633f),
  float3(0.2336449287f, 0.8367723485f, 0.9790359221f),
  float3(0.6391247265f, 0.4493507140f, 0.4442968520f),
  float3(0.4498084338f, 0.4281788393f, 0.8387843426f),
  float3(0.4617221066f, 0.4025249950f, 0.5317400227f),
  float3(0.2283070932f, 0.8667024408f, 0.3944511763f),
  float3(0.8686423674f, 0.3514442455f, 0.3009376871f),
  float3(0.4561867584f, 0.6396233311f, 0.1464020512f),
  float3(0.8434157662f, 0.5682984461f, 0.5822204098f),
  float3(0.3379975966f, 0.0817294524f, 0.2644603391f),
  float3(0.4594945054f, 0.5210836716f, 0.2796276083f),
  float3(0.1771902533f, 0.3277377500f, 0.0966192025f),
  float3(0.2712035576f, 0.8994619292f, 0.9284206759f),
  float3(0.1410653157f, 0.8783414068f, 0.8596058179f),
  float3(0.3029768905f, 0.4482880464f, 0.8201648240f),
  float3(0.2952526492f, 0.8497071519f, 0.1726950161f),
  float3(0.3137440653f, 0.0306490825f, 0.3313709364f),
  float3(0.3192157573f, 0.3405265548f, 0.8665817130f),
  float3(0.0233601686f, 0.3852952959f, 0.9729688912f),
  float3(0.0217756193f, 0.4384753783f, 0.8575746448f),
  float3(0.5957190243f, 0.0541722941f, 0.1895329569f),
  float3(0.4916864461f, 0.9419815539f, 0.1778794035f),
  float3(0.6364613771f, 0.6576617313f, 0.3387303629f),
  float3(0.9419362202f, 0.1969071879f, 0.1295044004f),
  float3(0.7692019339f, 0.0588093771f, 0.5619422776f),
  float3(0.1254520482f, 0.9804750036f, 0.7023502182f),
  float3(0.2202296170f, 0.5608206846f, 0.3684378277f),
  float3(0.0701899516f, 0.2767151785f, 0.4298845806f),
  float3(0.7252772445f, 0.0502138240f, 0.2084562891f),
  float3(0.3646451766f, 0.1747796578f, 0.4779791283f),
  float3(0.1715207637f, 0.8080296871f, 0.9626456051f),
  float3(0.7411410653f, 0.1542508542f, 0.9449019285f),
  float3(0.6941840614f, 0.0558966871f, 0.7666878887f),
  float3(0.6623411069f, 0.2818219639f, 0.8213221701f),
  float3(0.9262643258f, 0.7560344777f, 0.5905431196f),
  float3(0.4098744582f, 0.8231228065f, 0.2537074412f),
  float3(0.9010852033f, 0.3576850662f, 0.9691571526f),
  float3(0.2524156407f, 0.7840997517f, 0.7422420126f),
  float3(0.3115945018f, 0.3802933425f, 0.3162121230f),
  float3(0.8862781150f, 0.2529277478f, 0.6562000236f),
  float3(0.7239367375f, 0.0214521987f, 0.7693998389f),
  float3(0.9402198872f, 0.1462742923f, 0.8632011625f),
  float3(0.1722923425f, 0.2171545042f, 0.7055546979f),
};

// static const int kKernelCount = 14;
// static const float3 kKernels[kKernelCount] = {
// 	float3(+1.0f, +1.0f, +1.0f),
// 	float3(-1.0f, -1.0f, -1.0f),
// 	float3(-1.0f, +1.0f, +1.0f),
// 	float3(+1.0f, -1.0f, -1.0f),
// 	float3(+1.0f, +1.0f, -1.0f),
// 	float3(-1.0f, -1.0f, +1.0f),
// 	float3(-1.0f, +1.0f, -1.0f),
// 	float3(+1.0f, -1.0f, +1.0f),
// 	float3(-1.0f, 0.0f, 0.0f),
// 	float3(+1.0f, 0.0f, 0.0f),
// 	float3(0.0f, -1.0f, 0.0f),
// 	float3(0.0f, +1.0f, 0.0f),
// 	float3(0.0f, 0.0f, -1.0f),
// 	float3(0.0f, 0.0f, +1.0f)
// };

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

static const float kSurfaceEpsilon = 0.05f;
static const float kOcclusionRadius = 0.25f;
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

float3 getRandomNoise(float2 tex)
{
  int index = ((tex.x * 8) % 8) + ((tex.y * 8) % 8) * 8;
  return kRandomSamples[index] * 2.0f - 1.0f;
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
	return saturate(pow(access, 3.0f));
}