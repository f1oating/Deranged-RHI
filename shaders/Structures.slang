struct Transform {
    float4x4 ModelViewProjection;
    float4x4 TransInvModelView;
    float4x4 Model;
}

struct Light {
    float3 Directional; // in view space
}