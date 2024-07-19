cbuffer modelCB : register(b0)
{
    matrix modelMatrix;
};

cbuffer viewCB : register(b1)
{
    matrix viewMatrix;
};

cbuffer projCB : register(b2)
{
    matrix projMatrix;
};

cbuffer dataCB : register(b3)
{
    float elapsedTime;
    float padding1;
    float padding2;
    float padding3;
};

Texture2D Texture : register(t0);
sampler TextureSampler : register(s0);

struct vsoutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

struct vsinput {
    float4 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

float random( float2 p )
{
    float2 K1 = float2(
        23.14069263277926, // e^pi (Gelfond's constant)
         2.665144142690225 // 2^sqrt(2) (Gelfondâ€“Schneider constant)
    );
    return frac( cos( dot(p,K1) ) * 12345.6789 );
};

vsoutput vsmain(vsinput input)
{
	vsoutput output;
	
    output.position = mul(input.position, modelMatrix);
    float2 worldUv = (output.position.xz / 10) + 0.5;

    float randomVal = random(worldUv);

    float height = lerp(.5, 1, randomVal) * (1 - input.uv.y);
    height = lerp(height, 0, Texture.SampleLevel(TextureSampler, worldUv, 0).r);
    output.position.y = height;

    output.position.x += (sin(elapsedTime + randomVal) * 0.5 + 0.5) * (1 - input.uv.y) * lerp(.3, .45, randomVal) * height;
    output.position.z += lerp(-0.05, 0.05, randomVal) * (1 - input.uv.y);

    output.position.x += lerp(-0.1, 0.1, randomVal) * input.uv.y;
    output.position.z += lerp(-0.1, 0.1, randomVal) * input.uv.y;

    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projMatrix);

    input.normal = normalize(input.normal);
    output.normal = mul(input.normal, (float3x3)modelMatrix);
    output.normal = normalize(output.normal);

    output.uv = input.uv;

    return output;
}