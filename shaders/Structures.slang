struct Transform {
    column_major float4x4 ModelViewProjection;
    column_major float4x4 ModelView;
}

struct Light {
    float3 Directional; // in view space
}