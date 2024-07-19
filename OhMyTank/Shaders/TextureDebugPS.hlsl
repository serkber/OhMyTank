Texture2D Texture : register(t0);
sampler TextureSampler : register(s0);

struct vsoutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

float4 psmain(vsoutput input) : SV_TARGET
{
    return Texture.Sample(TextureSampler, input.uv);
}