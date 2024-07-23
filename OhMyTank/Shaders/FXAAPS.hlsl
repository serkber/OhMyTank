cbuffer data : register(b0)
{
    float elapsedTime;
    float mouseX;
    float square;
    float grassfieldSize;
    float grassPos;
    float grassAspect;
    float tankPos;
    float mouseY;
};

Texture2D Mask : register(t0);
Texture2D Frame : register(t1);
sampler TextureSampler : register(s0);

struct vsoutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
    float4 color : COLOR;
};

float4 psmain(vsoutput input) : SV_TARGET
{
    int iterations = 8;
    float offset = 0.0032 / (float)iterations;
    float2 offsetV = float2(0, offset);
    float2 offsetH = float2(offset, 0);

    float4 blurred = Frame.Sample(TextureSampler, input.uv);
    float4 frame = blurred;
    for(int i = 1; i < iterations + 1; i++)
    {
        float4 up = Frame.Sample(TextureSampler, input.uv + offsetV * i);
        float4 down = Frame.Sample(TextureSampler, input.uv - offsetV * i);
        float4 left = Frame.Sample(TextureSampler, input.uv - offsetH * i);
        float4 right = Frame.Sample(TextureSampler, input.uv + offsetH * i);

        float4 upRight = Frame.Sample(TextureSampler, input.uv + offsetV + offsetH * i * 0.75);
        float4 upLeft = Frame.Sample(TextureSampler, input.uv - offsetV - offsetH * i * 0.75);
        float4 downLeft = Frame.Sample(TextureSampler, input.uv - offsetH - offsetV * i * 0.75);
        float4 downRight = Frame.Sample(TextureSampler, input.uv + offsetH - offsetV * i * 0.75);

        blurred += (up + down + left + right + upRight + upLeft + downLeft + downRight);
    }

    blurred /= iterations * 8;

    float mask = Mask.Sample(TextureSampler, input.uv).r;
    return lerp(frame, blurred, mask);
}