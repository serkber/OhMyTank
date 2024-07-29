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