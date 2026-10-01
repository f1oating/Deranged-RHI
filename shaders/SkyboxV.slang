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
cbuffer Transform : register(b0) {
    column_major float4x4 projView;
};

[shader("vertex")]
Output main(Input input) {
    Output output;
    output.UV = input.Position;
    #ifdef VULKAN
    input.Position.y = -input.Position.y;
    #endif
    output.Position = mul(projView, float4(input.Position, 1.0)).xyww;
    return output;
}