struct vsoutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
    float4 color : COLOR;
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
	
    output.position = input.position * float4(2, 2, 0, 1);
    output.normal = input.normal;
    output.uv.x = input.uv.x;
    output.uv.y = -input.uv.y;
    output.color = input.color;

    return output;
}