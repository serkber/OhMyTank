cbuffer cbChangesEveryFrame : register(b0)
{
    matrix modelMatrix;
};

cbuffer cbNeverChanges : register(b1)
{
    matrix viewMatrix;
};

cbuffer cbChangeOnResize : register(b2)
{
    matrix projMatrix;
};

struct vsoutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uvs : UV;
};

struct vsinput {
    float4 position : POSITION;
    float3 normal : NORMAL;
    float2 uvs : UV;
};

vsoutput vsmain(vsinput input)
{
	vsoutput output;
	
    output.position = mul(input.position, modelMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projMatrix);

    input.normal = normalize(input.normal);
    output.normal = mul(input.normal, (float3x3)modelMatrix);
    output.normal = normalize(output.normal);

    output.uvs = input.uvs;

    return output;
}