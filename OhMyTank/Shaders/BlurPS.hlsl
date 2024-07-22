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

Texture2D Texture : register(t0);
sampler TextureSampler : register(s0);

struct vsoutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
    float4 color : COLOR;
};

float4 psmain(vsoutput input) : SV_TARGET
{
    float2 offsetV = float2(0, 0.001);
    float2 offsetH = float2(0.001, 0);
    float4 up = Texture.Sample(TextureSampler, input.uv + offsetV);
    float4 down = Texture.Sample(TextureSampler, input.uv - offsetV);
    float4 left = Texture.Sample(TextureSampler, input.uv - offsetH);
    float4 right = Texture.Sample(TextureSampler, input.uv + offsetH);
    float4 up2 = Texture.Sample(TextureSampler, input.uv + offsetV * 2);
    float4 down2 = Texture.Sample(TextureSampler, input.uv - offsetV * 2);
    float4 left2 = Texture.Sample(TextureSampler, input.uv - offsetH * 2);
    float4 right2 = Texture.Sample(TextureSampler, input.uv + offsetH * 2);

    return (up + down + left + right + up2 + down2 + left2 + right2) / 8;
}