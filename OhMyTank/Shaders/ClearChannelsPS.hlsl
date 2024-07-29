Texture2D Texture : register(t0);
sampler TextureSampler : register(s0);

#include "Shaders/Common/BasicVSOutput.hlsl"

float4 psmain(vsoutput input) : SV_TARGET
{
    float4 tex = Texture.Sample(TextureSampler, input.uv);
    return float4(0, 0, 0, 0);
}