cbuffer data : register(b0)
{
    float elapsedTime;
    float mouseX;
    float square;
    float grassfieldSize;
    float grassPos;
    float grassAspect;
    float camPos;
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
    float2 offsetV = float2(0, 0.002);
    float2 offsetH = float2(0.002, 0);
    float4 up = Frame.Sample(TextureSampler, input.uv + offsetV);
    float4 down = Frame.Sample(TextureSampler, input.uv - offsetV);
    float4 left = Frame.Sample(TextureSampler, input.uv - offsetH);
    float4 right = Frame.Sample(TextureSampler, input.uv + offsetH);
    float4 up2 = Frame.Sample(TextureSampler, input.uv + offsetV * 2);
    float4 down2 = Frame.Sample(TextureSampler, input.uv - offsetV * 2);
    float4 left2 = Frame.Sample(TextureSampler, input.uv - offsetH * 2);
    float4 right2 = Frame.Sample(TextureSampler, input.uv + offsetH * 2);

    float4 frame = Frame.Sample(TextureSampler, input.uv);
    float4 blurred = (up + down + left + right + up2 + down2 + left2 + right2) / 8;

    float mask = Mask.Sample(TextureSampler, input.uv).r;

    return lerp(frame, blurred, mask);
}