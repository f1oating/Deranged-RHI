static float2 verticies[] = {
    0.0, 0.5,
    0.5, -0.5,
    -0.5, -0.5
};

[shader("vertex")]
float4 main(uint idx : SV_VertexID) : SV_Position {
    return float4(verticies[idx], 0.5, 1.0);
}