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
    float4x4 worldView = mul(TransformBuffer.View, TransformBuffer.World);
    float4x4 worldViewProjection = mul(TransformBuffer.Projection, worldView);

    Output output;

    output.Position = mul(worldViewProjection, float4(input.Position, 1.0));
    output.UV = input.UV;

    output.Normal = input.Normal;
    output.Tangent = input.Tangent;
    output.Bitangent = input.Bitangent;

    return output;
}