struct Input {
    float2 Position : POSITION;
};

[shader("vertex")]
float4 main(Input input) : SV_Position {
    #ifdef VULKAN
    input.Position.y = -input.Position.y;
    #endif
    return float4(input.Position, 0.5, 1.0);
}