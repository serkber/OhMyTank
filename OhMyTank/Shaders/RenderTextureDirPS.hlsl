#include "Shaders/Common/BasicVSOutput.hlsl"

float4 psmain(vsoutput input) : SV_TARGET
{
	float2 val2 = smoothstep(0, 0.4, input.uv) * smoothstep(1, 0.6, input.uv);
	float val = val2.x * val2.y;
	return float4(0, input.color.gb,val);
}