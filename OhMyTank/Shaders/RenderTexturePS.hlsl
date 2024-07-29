#include "Shaders/Common/BasicVSOutput.hlsl"

float4 psmain(vsoutput input) : SV_TARGET
{
	float val = max (step(0.85, input.uv.x), (1 - step(0.15, input.uv.x)));
	return float4(0, 0, 0, val);
	//float4 dir = float4(0, input.color.gb, 0.1);
	//float4 tracks = float4(0, 0, 0, 0.1);
    //return lerp(tracks, dir, val);
}