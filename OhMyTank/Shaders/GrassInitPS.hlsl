struct vsoutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
    float4 color : COLOR;
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

float4 psmain(vsoutput input) : SV_TARGET
{
    float sides = smoothstep(1, 0.7, input.uv.x) * smoothstep(0, 0.3, input.uv.x) * 2;
    sides += gnoise(input.uv * float2(10, 40)) - 0.5;
    sides = saturate(sides);
    
    float middle = saturate(gnoise(input.uv * float2(5, 20)) * 2);
    return sides * middle;
}