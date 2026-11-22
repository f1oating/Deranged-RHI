struct Transform {
    float4x4 World;
    float4x4 View;
    float4x4 Projection;

    float4x4 TransInvWorldView;
}

struct Light {
    float3 Directional; // in view space
}