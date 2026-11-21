import "Structures.slang";

struct Input {
    float3 Position : Position;
    float3 Color : COLOR;
};

struct Output {
    float4 Position : SV_Position;
    float3 Color : COLOR;
};

[vk::binding(0, 0)]
ConstantBuffer<Transform> TransformBuffer : register(b0);

[shader("vertex")]
Output main(Input input) {
    Output output;
    output.Position = mul(TransformBuffer.ModelViewProjection, float4(input.Position, 1.0));
    output.Color = input.Color;
    return output;
}