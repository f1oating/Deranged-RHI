struct Input {
    float4 Position : SV_Position;
    float3 Color : COLOR;
};

[shader("fragment")]
float4 main(Input input) : SV_Target {
    return float4(input.Color, 1.0);
}