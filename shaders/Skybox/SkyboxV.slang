import "Structures.slang";

struct Input {
    float3 Position : POSITION;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
};

struct Output {
    float4 Position : SV_Position;
    float3 UV : TEXCOORD;
}

[vk::binding(0, 0)]
ConstantBuffer<Transform> TransformBuffer : register(b0);

[shader("vertex")]
Output main(Input input) {
    float4x4 worldView = mul(TransformBuffer.View, TransformBuffer.World);
    float4x4 worldViewProjection = mul(TransformBuffer.Projection, worldView);

    Output output;
    output.UV = input.Position;
    output.Position = mul(worldViewProjection, float4(input.Position, 1.0)).xyww;

    return output;
}