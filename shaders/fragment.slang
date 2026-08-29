cbuffer Color : register(b0) {
    float4 color;
};

[shader("fragment")]
float4 main() : SV_Target {
    return color;
}