import "Structures.slang";

static const float3 AMBIENT = float3(0.1, 0.1, 0.1);

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

    input.Normal = normalize(input.Normal);
    input.Tangent = normalize(input.Tangent);
    input.Bitangent = normalize(input.Bitangent);

    normal = normalize(mul(transpose(float3x3(input.Tangent, input.Bitangent, input.Normal)), normal));
    normal = normalize(mul(float3x3(TransformBuffer.TransInvWorldView), normal));

    float3 diffuse = Diffuse.Sample(Sampler, input.UV).xyz;

    float angle = max(0.0, dot(normalize(normal), normalize(LightBuffer.Directional)));
    float3 lightColor = float3(0.1, 0.1, 0.1) * angle;

    return float4(diffuse * AMBIENT + lightColor, 1.0);
}