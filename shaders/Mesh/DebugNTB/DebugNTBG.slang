import "Structures.slang";

struct InputVertex {
    float4 Position : SV_Position;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
}

struct OutputVertex {
    float4 Position : SV_Position;
    float3 Color : COLOR;
};

[vk::binding(0, 0)]
ConstantBuffer<Transform> TransformBuffer : register(b0);

void ProccessVertex(InputVertex vertex, inout LineStream<OutputVertex> output) {
    output.Append({ mul(TransformBuffer.Projection, vertex.Position), { 0.0f, 0.0f, 1.0f }});
    output.Append({ mul(TransformBuffer.Projection, float4(vertex.Position.xyz + normalize(vertex.Normal) * 0.1f, 1.0)), { 0.0f, 0.0f, 1.0f }});
    output.RestartStrip();

    output.Append({ mul(TransformBuffer.Projection, vertex.Position), { 0.0f, 1.0f, 0.0f }});
    output.Append({ mul(TransformBuffer.Projection, float4(vertex.Position.xyz + normalize(vertex.Tangent) * 0.1f, 1.0)), { 0.0f, 1.0f, 0.0f }});
    output.RestartStrip();

    output.Append({ mul(TransformBuffer.Projection, vertex.Position), { 1.0f, 0.0f, 0.0f }});
    output.Append({ mul(TransformBuffer.Projection, float4(vertex.Position.xyz + normalize(vertex.Bitangent) * 0.1f, 1.0)), { 1.0f, 0.0f, 0.0f }});
    output.RestartStrip();
}

[shader("geometry")]
[maxvertexcount(6)]
void main(triangle InputVertex[3] input, inout LineStream<OutputVertex> output) {
    for (int i = 0; i < 3; i++) {
        ProccessVertex(input[i], output);
    }
}