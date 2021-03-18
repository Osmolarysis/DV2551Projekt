uint wang_hash(uint seed) //Random generator
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}

cbuffer dtBuffer {
    float dt;
    float padding[63];
};

RWBuffer<float3> rotationBuffer;

static const float PI = 3.14159265f;

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float delta = dt;

    // Randomise
    float randomx = wang_hash(DTid.x);
    float randomy = wang_hash(randomx);
    float randomz = wang_hash(randomy);

    // Normalise
    randomx = randomx * (1.0f / 4294967296.0f);
    randomy = randomy * (1.0f / 4294967296.0f);
    randomz = randomz * (1.0f / 4294967296.0f);

    // Update-ise
    float3 updatedCube = rotationBuffer[DTid.x];
    updatedCube.x += randomx * PI * delta;
    updatedCube.y += randomy * PI * delta;
    updatedCube.z += randomz * PI * delta;

    rotationBuffer[DTid.x] = updatedCube;
}