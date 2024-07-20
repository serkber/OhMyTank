cbuffer data : register(b0)
{
	float elapsedTime;
    float mouseX;
    float square;
    float grassFieldSize;
    float grassPos;
};

struct vsoutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
    float4 color : COLOR;
};

float4 psmain(vsoutput input) : SV_TARGET
{
	float val;
	if(square == 0)
	{
    	val = length(input.uv * 2 - 1);
	}
	else
	{
		float smooth = 0.5;
		float2 sq = smoothstep(smooth, 0.0, input.uv);
		sq = max(sq, smoothstep(1 - smooth, 1.0, input.uv));
		val = max(sq.r, sq.g);

	}
    return float4(val, 0, 0, 1);
}