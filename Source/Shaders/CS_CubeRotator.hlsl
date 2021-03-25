cbuffer matrixBuffer : register(b0)
{
    float4x4 viewMatrix;
    float4x4 projMatrix;
    float dt;
    float3 smallPadd;
    float4 matrixPadd[7];
}

struct transformation {
    float4 rotation;
    float4 translation;
};

RWStructuredBuffer <transformation> transformBuffer_1 : register(u0);
RWStructuredBuffer <transformation> transformBuffer_2 : register(u1);

static const float PI = 3.14159265f;

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    //Read data
    float delta = dt;
    transformation transform = transformBuffer_1[DTid.x];

    // Update
    transform.rotation.x += PI * delta;
    transform.rotation.y += PI * delta;
    transform.rotation.z += PI * delta;

   //Write data
    transformBuffer_2[DTid.x] = transform;
}