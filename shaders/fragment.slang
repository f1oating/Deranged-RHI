struct Input {
    float4 Position : SV_Position;
    float2 UV;
}

[vk::binding(1, 0)]
Texture2D<float4> Albedo : register(t0);

[vk::binding(2, 0)]
SamplerState Sampler : register(s0);

[shader("fragment")]
float4 main(Input input) : SV_Target {
    return Albedo.Sample(Sampler, input.UV);
}