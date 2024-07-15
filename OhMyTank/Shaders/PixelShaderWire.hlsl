struct vsoutput {
    float4 position : SV_POSITION;
};

float4 psmain(vsoutput input) : SV_TARGET
{
    return float4(0, 0, 0, 1);
}