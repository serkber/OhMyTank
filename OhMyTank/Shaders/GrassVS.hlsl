cbuffer view : register(b1)
{
    matrix viewMatrix;
};

cbuffer projection : register(b2)
{
    matrix projMatrix;
};

cbuffer data : register(b3)
{
    float elapsedTime;
    float mouseX;
    float square;
    float grassFieldSize;
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

struct vsinput {
    float4 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
    float4 color : COLOR;
    uint instanceID : SV_InstanceID;
    matrix iModel : INSTANCE_MODEL;
};

float random( float2 p )
{
    float2 K1 = float2(23.14069263277926, 2.665144142690225);
    return frac( cos( dot(p,K1) ) * 12345.6789);
};

float gnoise(float2 n) {
    const float2 d = float2(0.0, 1.0);
    float2  b = floor(n), 
    f = smoothstep(d.xx, d.yy, frac(n));

    float x = lerp(random(b), random(b + d.yx), f.x),
          y = lerp(random(b + d.xy), random(b + d.yy), f.x);

    return lerp( x, y, f.y );
};

float inverseLerp(float from, float to, float val){
    return (val - from) / (to - from);
}

vsoutput vsmain(vsinput input)
{
	vsoutput output = (vsoutput)0;
	
    output.position = mul(input.position, input.iModel);
    float2 bladePos = float2( input.iModel._41, input.iModel._43 ) + input.color.rg;

    float randomVal = random(bladePos);

// Blade offset
    output.position.x += lerp(-0.5, 0.7, randomVal);
    output.position.z += lerp(-0.3, 1.1, randomVal);

// Tip height
    output.color = gnoise(bladePos / 2);

    float2 grassFieldDimensions = float2(grassFieldSize, grassFieldSize * grassAspect);
    float2 worldUv = output.position.xz + grassFieldDimensions / 2;
    worldUv /= grassFieldDimensions;
    worldUv -= float2(0, grassPos / (grassFieldSize * grassAspect));

    float heightMap = 1 - Texture.SampleLevel(TextureSampler, worldUv, 0).r;
    float height = lerp(.5, 1.5, randomVal * output.color.r) * input.uv.y;
    height = lerp(height, 0, heightMap);
    output.position.y = height;

    output.color.a = heightMap;

// Tip swing
    float noise = gnoise(bladePos + elapsedTime);
    float swing = sin(elapsedTime + noise * 4);
    swing += sin(elapsedTime * 3.7 + randomVal);

    float swingAmount = (swing * 0.3 + 0.5) * input.uv.y * lerp(.3, .45, randomVal) * height;
    output.position.x += swingAmount;
    output.color.g = swingAmount;

    input.normal = normalize(input.normal);
    output.normal = input.normal;
    output.normal = normalize(output.normal);

// Curve
    float val = (output.position.z - camPos);
    val = inverseLerp(5, 300, val);
    val *= val;
    output.position.y += val * 60;

// View proj transformation
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projMatrix);

    output.uv = input.uv;

    return output;
}