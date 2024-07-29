#include "Shaders/Common/CBufferModel.hlsl"

#include "Shaders/Common/BasicVSOutput.hlsl"
#include "Shaders/Common/BasicVSInput.hlsl"

vsoutput vsmain(vsinput input)
{
	vsoutput output;
	
    input.position *= float4(2, 2, 0, 1);
    output.position = mul(input.position, modelMatrix);
    output.normal = input.normal;
    output.uv.x = input.uv.x;
    output.uv.y = 1 - input.uv.y;
    output.color = input.color;

    return output;
}