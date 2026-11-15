import "Structures.slang";

static const float3 AMBIENT = float3(0.2, 0.2, 0.2);

struct Input {
    float4 Position : SV_Position;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
}

[vk::binding(1, 0)]
Texture2D<float4> Diffuse : register(t0);

[vk::binding(2, 0)]
SamplerState Sampler : register(s0);

[vk::binding(3, 0)]
ConstantBuffer<Light> LightBuffer : register(b1);

[shader("fragment")]
float4 main(Input input) : SV_Target {
    float3 diffuse = Diffuse.Sample(Sampler, input.UV).xyz;

    float angle = max(0.0, dot(normalize(input.Normal), normalize(-LightBuffer.Directional)));
    float3 lightColor = float3(0.3, 0.3, 0.3) * angle;

    return float4(diffuse * AMBIENT + lightColor, 1.0);
}