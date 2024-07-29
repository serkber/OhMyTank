#include "Shaders/Common/BasicVSOutput.hlsl"

float4 psmain(vsoutput input) : SV_TARGET
{    
    float noise = lerp(0.6, 1, input.color.r);
    float4 col1 = float4(0.2, 0.2, 0.15, 1);
    float4 col2 = float4(0.5, 0.5, 0.3, 1);
    float4 col = lerp(col1, col2, input.uv.y);
    float val = saturate(dot(input.normal, float3(1, 1, -1)) * 0.7);
    val *= input.uv.y * noise;
    float4 col3 = float4(0.7, 0.7, 0.35, 1);
    col = lerp(col, col3, val);
    col *= noise;
    col = lerp(float4(0.2, 0.15, 0.05, 1), col, input.color.a);
    return col;
}