cbuffer model : register(b0)
{
    matrix modelMatrix;
};

cbuffer data : register(b3)
{
    float elapsedTime;
    float mouseX;
    float square;
    float grassFieldSize;
    float grassPos;
    float grassAspect;
};

struct vsoutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

struct vsinput {
    float4 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
    float4 color : COLOR;
};

vsoutput vsmain(vsinput input)
{
	vsoutput output;
	
    output.position = mul(input.position, modelMatrix);

//Aspect correction
    output.position.xy -= float2(modelMatrix._41, modelMatrix._42);
    output.position.y /= grassAspect;
    output.position.xy += float2(modelMatrix._41, modelMatrix._42);

    output.normal = input.normal;
    output.uv = input.uv;

    return output;
}