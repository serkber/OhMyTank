#include "Shaders/Common/BasicVSInput.hlsl"
#include "Shaders/Common/BasicVSOutput.hlsl"

vsoutput vsmain(vsinput input)
{
	vsoutput output;
	
    output.position = input.position * float4(2, 2, 0, 1);
    output.normal = input.normal;
    output.uv.x = input.uv.x;
    output.uv.y = -input.uv.y;
    output.color = input.color;

    return output;
}