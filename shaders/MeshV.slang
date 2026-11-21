import "Structures.slang";

struct Input {
    float3 Position : POSITION;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
};

struct Output {
    float4 Position : SV_Position;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
}

[vk::binding(0, 0)]
ConstantBuffer<Transform> TransformBuffer : register(b0);

[shader("vertex")]
Output main(Input input) {
    Output output;

    output.Position = mul(TransformBuffer.ModelViewProjection, float4(input.Position, 1.0));
    output.UV = input.UV;
    output.Normal = mul(float3x3(TransformBuffer.TransInvModelView), input.Normal);
    output.Tangent = mul(float3x3(TransformBuffer.TransInvModelView), input.Tangent);
    output.Bitangent = mul(float3x3(TransformBuffer.TransInvModelView), input.Bitangent);

    return output;
}