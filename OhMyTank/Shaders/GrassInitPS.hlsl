#include "Shaders/Common/BasicVSOutput.hlsl"
#include "Shaders/Common/Utils.hlsl"
#include "Shaders/Common/CBufferData.hlsl"

float4 psmain(vsoutput input) : SV_TARGET
{
    float2 uvs = input.uv + d_grassfieldPos;
    float noise = gnoise(uvs * 20) * 0.4 + 0.6;
    noise *= noise;
    float rand = random(uvs) * 0.7 + 0.3;
    noise *= rand;
    return float4(noise, 0, 0, 1);
}