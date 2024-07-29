#include "Shaders/Common/CBufferData.hlsl"
#include "Shaders/Common/CBufferModel.hlsl"
#include "Shaders/Common/CBufferView.hlsl"
#include "Shaders/Common/CBufferProjection.hlsl"

#include "Shaders/Common/BasicVSOutput.hlsl"
#include "Shaders/Common/BasicVSInput.hlsl"

vsoutput vsmain(vsinput input)
{
	vsoutput output;
	
    output.position = mul(input.position, modelMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projMatrix);

    input.normal = normalize(input.normal);
    output.normal = mul(input.normal, (float3x3)modelMatrix);
    output.normal = normalize(output.normal);

    output.uv = input.uv;

    output.color = input.color;

    return output;
}