#include "Shaders/Common/CBufferData.hlsl"

Texture2D Texture : register(t0);
sampler TextureSampler : register(s0);

#include "Shaders/Common/BasicVSOutput.hlsl"

float4 psmain(vsoutput input) : SV_TARGET
{   
    float2 offsetV = float2(0, 0.0002);
    float2 offsetH = float2(0.0002, 0);
    float4 blurred = Texture.Sample(TextureSampler, input.uv);
    int iterations = 8;
    for(int i = 1; i < iterations + 1; i++)
    {
        float4 up = Texture.Sample(TextureSampler, input.uv + offsetV * i);
        float4 down = Texture.Sample(TextureSampler, input.uv - offsetV * i);
        float4 left = Texture.Sample(TextureSampler, input.uv - offsetH * i);
        float4 right = Texture.Sample(TextureSampler, input.uv + offsetH * i);

        float4 upRight = Texture.Sample(TextureSampler, input.uv + offsetV + offsetH * i * 0.75);
        float4 upLeft = Texture.Sample(TextureSampler, input.uv - offsetV - offsetH * i * 0.75);
        float4 downLeft = Texture.Sample(TextureSampler, input.uv - offsetH - offsetV * i * 0.75);
        float4 downRight = Texture.Sample(TextureSampler, input.uv + offsetH - offsetV * i * 0.75);

        blurred += (up + down + left + right + upRight + upLeft + downLeft + downRight);
    }

    blurred /= iterations * 8;

    blurred = smoothstep(0, 0.1, blurred);

    return blurred;
}