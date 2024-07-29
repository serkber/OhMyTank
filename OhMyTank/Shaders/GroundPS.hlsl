Texture2D Texture : register(t0);
sampler TextureSampler : register(s0);

#include "Shaders/Common/BasicVSOutput.hlsl"

float4 psmain(vsoutput input) : SV_TARGET
{
	float tex = Texture.Sample(TextureSampler, input.uv).r;
	float3 col1 = float3(0.22, 0.18, 0.12);
	return float4(col1, 1);
	float3 col2 = float3(0.3, 0.3, 0.2);
	float3 col = lerp(col1, col2, tex);

    return float4(col, 1);
}