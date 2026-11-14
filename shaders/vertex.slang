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
cbuffer Transform : register(b0) {
    column_major float4x4 ModelViewProjection;
    column_major float4x4 ModelView;
};

[shader("vertex")]
Output main(Input input) {
    #ifdef VULKAN
    input.Position.y = -input.Position.y;
    #endif
    Output output;
    output.Position = mul(ModelViewProjection, float4(input.Position, 1.0));
    output.UV = input.UV;
    input.Normal.y = -input.Normal.y;
    output.Normal = mul(float3x3(ModelView), input.Normal);
    return output;
}