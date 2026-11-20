import "Structures.slang";

struct Input {
    float3 Position : POSITION;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
};

struct Output {
    float4 Position : SV_Position;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
}

[vk::binding(0, 0)]
ConstantBuffer<Transform> TransformBuffer : register(b0);

[shader("vertex")]
Output main(Input input) {
    Output output;
    output.Position = mul(TransformBuffer.ModelViewProjection, float4(input.Position, 1.0));
    output.UV = input.UV;
    output.Normal = mul(TransformBuffer.TransInvModelView, float4(input.Normal, 1.0)).xyz;

    return output;
}