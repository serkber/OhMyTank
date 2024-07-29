#include "Shaders/Common/CBufferData.hlsl"

Texture2D Texture : register(t0);
sampler TextureSampler : register(s0);

#include "Shaders/Common/BasicVSOutput.hlsl"

float4 psmain(vsoutput input) : SV_TARGET
{
    float2 offsetV = float2(0, 0.0005);
    float2 offsetH = float2(0.0005, 0);
    float4 up = Texture.Sample(TextureSampler, input.uv + offsetV);
    float4 down = Texture.Sample(TextureSampler, input.uv - offsetV);
    float4 left = Texture.Sample(TextureSampler, input.uv - offsetH);
    float4 right = Texture.Sample(TextureSampler, input.uv + offsetH);

    float sil = abs(right.r - left.r) + abs(up.r - down.r);
    return smoothstep(0, 0.2, sil);
}