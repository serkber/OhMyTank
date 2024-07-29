#include "Shaders/Common/CBufferData.hlsl"
#include "Shaders/Common/CBufferModel.hlsl"

#include "Shaders/Common/BasicVSInput.hlsl"
#include "Shaders/Common/BasicVSOutput.hlsl"

vsoutput vsmain(vsinput input)
{
	vsoutput output = (vsoutput)0;
	
    output.position = mul(input.position, modelMatrix);

//Aspect correction
    // output.position.xy -= float2(modelMatrix._41, modelMatrix._42);
    // output.position.y /= grassAspect;
    // output.position.xy += float2(modelMatrix._41, modelMatrix._42);

    output.normal = input.normal;
    output.uv = input.uv;
    output.color.gb = normalize(float2(-modelMatrix._21, modelMatrix._22));

    return output;
}