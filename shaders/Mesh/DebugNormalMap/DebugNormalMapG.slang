import "Structures.slang";

struct InputVertex {
    float4 Position : SV_Position;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
};

struct OutputVertex {
    float4 Position : SV_Position;
    float3 Color : COLOR;
};

[vk::binding(0, 0)]
ConstantBuffer<Transform> TransformBuffer : register(b0);

[vk::binding(1, 0)]
Texture2D<float4> Normal : register(t0);

[vk::binding(2, 0)]
SamplerState Sampler : register(s0);

void ProccessVertex(InputVertex vertex, inout LineStream<OutputVertex> output) {
    float3 normal = Normal.SampleLevel(Sampler, vertex.UV, 0).xyz;
    normal = 2.0 * normal - 1.0;

    vertex.Normal = normalize(vertex.Normal);
    vertex.Tangent = normalize(vertex.Tangent);
    vertex.Bitangent = normalize(vertex.Bitangent);

    normal = normalize(mul(transpose(float3x3(vertex.Tangent, vertex.Bitangent, vertex.Normal)), normal));
    normal = normalize(mul(float3x3(TransformBuffer.TransInvWorldView), normal));

    output.Append({ mul(TransformBuffer.Projection, vertex.Position), { 0.0f, 1.0f, 0.0f }});
    output.Append({ mul(TransformBuffer.Projection, float4(vertex.Position.xyz + normal * 0.1f, 1.0)), { 0.0f, 1.0f, 0.0f }});
    output.RestartStrip();
}

[shader("geometry")]
[maxvertexcount(6)]
void main(triangle InputVertex[3] input, inout LineStream<OutputVertex> output) {
    for (int i = 0; i < 3; i++) {
        ProccessVertex(input[i], output);
    }
}