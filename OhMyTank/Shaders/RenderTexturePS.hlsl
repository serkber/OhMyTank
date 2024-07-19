struct vsoutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

float4 psmain(vsoutput input) : SV_TARGET
{
    return float4(1, 0, 0, 1);
}