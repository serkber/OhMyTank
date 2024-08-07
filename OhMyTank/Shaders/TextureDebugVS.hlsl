#include "Shaders/Common/CBufferModel.hlsl"

#include "Shaders/Common/BasicVSInput.hlsl"
#include "Shaders/Common/BasicVSOutput.hlsl"

vsoutput vsmain(vsinput input)
{
	vsoutput output;
	
    output.position = mul(input.position, modelMatrix);
    output.normal = input.normal;
    output.uv = input.uv;
    output.color = input.color;

    return output;
}