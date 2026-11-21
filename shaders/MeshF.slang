import "Structures.slang";

static const float3 AMBIENT = float3(0.3, 0.3, 0.3);

struct Input {
    float4 Position : SV_Position;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
}

[vk::binding(0, 0)]
ConstantBuffer<Transform> TransformBuffer : register(b0);

[vk::binding(1, 0)]
Texture2D<float4> Diffuse : register(t0);

[vk::binding(2, 0)]
Texture2D<float4> Normal : register(t1);

[vk::binding(3, 0)]
SamplerState Sampler : register(s0);

[vk::binding(4, 0)]
ConstantBuffer<Light> LightBuffer : register(b1);

[shader("fragment")]
float4 main(Input input) : SV_Target {
    float3 normal = Normal.Sample(Sampler, input.UV).xyz;
    normal = 2.0 * normal - 1.0;

    float3 nor = normalize(input.Normal);
    float3 tan = normalize(input.Tangent);
    float3 bit = normalize(input.Bitangent);

    normal = normalize(mul(float3x3(tan, bit, nor), normal));

    float3 diffuse = Diffuse.Sample(Sampler, input.UV).xyz;

    float angle = max(0.0, dot(normalize(normal), normalize(LightBuffer.Directional)));
    float3 lightColor = float3(0.2, 0.2, 0.2) * angle;

    return float4(diffuse * AMBIENT + lightColor, 1.0);
}