struct Input {
    float2 Position : POSITION;
};

[vk::binding(0, 0)]
cbuffer ViewProj : register(b0) {
    float4x4 viewProj;
};

[shader("vertex")]
float4 main(Input input) : SV_Position {
    #ifdef VULKAN
    input.Position.y = -input.Position.y;
    #endif
    return float4(input.Position, 0.5, 1.0);
}