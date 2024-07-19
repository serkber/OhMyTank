cbuffer cbChangesEveryFrame : register(b0)
{
    matrix modelMatrix;
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
    output.normal = input.normal;
    output.uvs = input.uvs;

    return output;
}