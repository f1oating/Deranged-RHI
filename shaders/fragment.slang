struct Input {
    float4 Position : SV_Position;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
}

[vk::binding(1, 0)]
Texture2D<float4> Albedo : register(t0);

[vk::binding(2, 0)]
SamplerState Sampler : register(s0);

[vk::binding(3, 0)]
ConstantBuffer<float3> Light : register(b1);

[shader("fragment")]
float4 main(Input input) : SV_Target {
    float lightCof = max(0.0, dot(normalize(input.Normal), normalize(-Light)));
    return Albedo.Sample(Sampler, input.UV) * float4(0.3, 0.3, 0.3, 1.0) + float4(float3(0.2, 0.2, 0.2) * lightCof, 0.0);
}