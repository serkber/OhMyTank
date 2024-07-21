cbuffer data : register(b0)
{
    float elapsedTime;
    float mouseX;
    float square;
    float grassFieldSize;
    float grassPos;
    float mouseY;
};

Texture2D Texture : register(t0);
sampler TextureSampler : register(s0);

struct vsoutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
    float4 color : COLOR;
};

float4 psmain(vsoutput input) : SV_TARGET
{
    float2 offset = float2(-0.00025, 0.0005);
    float4 sample1 = Texture.Sample(TextureSampler, input.uv + offset);
    float4 sample2 = Texture.Sample(TextureSampler, input.uv + offset * 2);
    float4 sample3 = Texture.Sample(TextureSampler, input.uv - offset);
    float4 sample4 = Texture.Sample(TextureSampler, input.uv - offset * 2);
    return (sample1 + sample2 + sample3 + sample4) / 4;
}