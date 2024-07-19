struct vsoutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

float4 psmain(vsoutput input) : SV_TARGET
{
    float4 col1 = float4(0.1, 0.2, 0.0, 1);
    float4 col2 = float4(0.4, 0.4, 0.2, 1);
    return lerp(col2, col1, input.uv.y);
    float val = saturate(dot(input.normal, float3(1, 1, -1)) * 0.7);
    val += saturate(dot(input.normal, float3(-1, 0, -1))) * 0.2;
    return float4(val, val, val, 1);
}