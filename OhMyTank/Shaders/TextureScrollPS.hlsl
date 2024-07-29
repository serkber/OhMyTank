Texture2D Texture : register(t0);
sampler TextureSampler : register(s0);

#include "Shaders/Common/BasicVSOutput.hlsl"

float4 psmain(vsoutput input) : SV_TARGET
{
    float2 mask = smoothstep(1, 0.95, input.uv) * smoothstep(0, 0.05, input.uv);
    return float4(Texture.Sample(TextureSampler, input.uv).rgb, mask.r * mask.g);
}