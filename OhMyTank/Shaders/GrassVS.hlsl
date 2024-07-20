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
    float mouseX;
    float padding2;
    float grassFieldSize;
};

Texture2D Texture : register(t0);
sampler TextureSampler : register(s0);

struct vsoutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
    float4 color : COLOR;
};

struct vsinput {
    float4 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
    float4 color : COLOR;
};

float random( float2 p )
{
    float2 K1 = float2(
        23.14069263277926, // e^pi (Gelfond's constant)
         2.665144142690225 // 2^sqrt(2) (Gelfondâ€“Schneider constant)
    );
    return frac( cos( dot(p,K1) ) * 12345.6789 );
};

float gnoise(float2 n) {
    const float2 d = float2(0.0, 1.0);
    float2  b = floor(n), 
    f = smoothstep(d.xx, d.yy, frac(n));

    //float2 f = frac(n);
    //f = f*f*(3.0-2.0*f);

    float x = lerp(random(b), random(b + d.yx), f.x),
          y = lerp(random(b + d.xy), random(b + d.yy), f.x);

    return lerp( x, y, f.y );
};

vsoutput vsmain(vsinput input)
{
	vsoutput output;
	
    output.position = mul(input.position, modelMatrix);
    float2 bladePos = float2( modelMatrix._41, modelMatrix._43 ) + input.color.rg;

    float randomVal = random(bladePos);

// Blade offset
    output.position.x += lerp(-0.5, 0.7, randomVal);
    output.position.z += lerp(-0.3, 1.1, randomVal);

// Tip height
    output.color = gnoise(bladePos / 2);

    float2 worldUv = ((output.position.xz + grassFieldSize / 2) / grassFieldSize);
    float heightMap = 1 - Texture.SampleLevel(TextureSampler, worldUv, 0).r;
    float height = lerp(.3, 1, randomVal * output.color.r) * input.uv.y;
    height = lerp(height, 0, heightMap);
    output.position.y = height;

    output.color.a = heightMap;

// Tip swing
    float noise = gnoise(bladePos + elapsedTime);
    float swing = sin(elapsedTime + noise * 4);
    swing += sin(elapsedTime * 3.7 + randomVal);
    output.position.x += (swing * 0.3 + 0.5) * input.uv.y * lerp(.3, .45, randomVal) * height;

    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projMatrix);

    input.normal = normalize(input.normal);
    output.normal = mul(input.normal, (float3x3)modelMatrix);
    output.normal = normalize(output.normal);

    output.uv = input.uv;

    return output;
}