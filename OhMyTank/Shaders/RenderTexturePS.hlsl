struct vsoutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

float4 psmain(vsoutput input) : SV_TARGET
{
    float val = length(input.uv * 2 - 1);
    return float4(val, 0, 0, 1);
}