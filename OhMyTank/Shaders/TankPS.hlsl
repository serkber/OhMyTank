Texture2D Texture : register(t0);
sampler TextureSampler : register(s0);

struct vsoutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

float4 psmain(vsoutput input) : SV_TARGET
{
	float light = saturate(dot(input.normal, float3(1, 1, -1)) * 0.7);
    light += saturate(dot(input.normal, float3(-1, 0, -1))) * 0.2;
	float3 color = Texture.Sample(TextureSampler, input.uv).rgb;
	
	return float4(color * light, 1);

    float val = saturate(dot(input.normal, float3(1, 1, -1)) * 0.7);
    val += saturate(dot(input.normal, float3(-1, 0, -1))) * 0.2;
    return float4(val, val, val, 1);
}