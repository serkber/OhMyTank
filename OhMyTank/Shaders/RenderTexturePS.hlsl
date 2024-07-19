struct vsoutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

float4 psmain(vsoutput input) : SV_TARGET
{
    float val = 1 - length(input.uv - 0.5) * 2;
    return float4(1, 1, 1, val);
}