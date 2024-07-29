struct vsinput {
    float4 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
    float4 color : COLOR;
    uint instanceID : SV_InstanceID;
    matrix iModel : INSTANCE_MODEL;
};