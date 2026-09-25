struct Input {
    float3 Position : POSITION;
    float2 UV : TEXCOORD;
};

struct Output {
    float4 Position : SV_Position;
    float2 UV : TEXCOORD;
}

[vk::binding(0, 0)]
cbuffer Transform : register(b0) {
    column_major float4x4 projView;
};

[shader("vertex")]
Output main(Input input) {
    #ifdef VULKAN
    input.Position.y = -input.Position.y;
    #endif
    Output output;
    output.Position = mul(projView, float4(input.Position, 1.0));
    output.UV = input.UV;
    return output;
}