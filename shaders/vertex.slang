struct Input {
    float3 Position : POSITION;
};

[vk::binding(0, 0)]
cbuffer ProjView : register(b0) {
    column_major float4x4 projView;
};

[shader("vertex")]
float4 main(Input input) : SV_Position {
    #ifdef VULKAN
    input.Position.y = -input.Position.y;
    #endif
    return mul(projView, float4(input.Position, 1.0));
}