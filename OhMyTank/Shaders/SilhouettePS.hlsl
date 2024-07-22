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
    float2 offsetV = float2(0, 0.0005);
    float2 offsetH = float2(0.0005, 0);
    float4 up = Texture.Sample(TextureSampler, input.uv + offsetV);
    float4 down = Texture.Sample(TextureSampler, input.uv - offsetV);
    float4 left = Texture.Sample(TextureSampler, input.uv - offsetH);
    float4 right = Texture.Sample(TextureSampler, input.uv + offsetH);

    float sil = abs(right.r - left.r) + abs(up.r - down.r);
    return smoothstep(0, 0.2, sil);
}