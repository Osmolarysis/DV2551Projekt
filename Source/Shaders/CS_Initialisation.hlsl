uint wang_hash(uint seed) //Random generator
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}

struct transformation {
    float4 rotation;
    float4 translation;
};

RWStructuredBuffer <transformation> transformBuffer_1 : register(u0);
RWStructuredBuffer <transformation> transformBuffer_2 : register(u1);

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    // Randomise
    float randomx = wang_hash(DTid.x);
    float randomy = wang_hash(randomx);
    float randomz = wang_hash(randomy);

    // Normalise
    randomx = randomx * (1.0f / 4294967296.0f);
    randomy = randomy * (1.0f / 4294967296.0f);
    randomz = randomz * (1.0f / 4294967296.0f);

    //Write
    transformation transform;
    transform.rotation = float4(randomx, randomy, randomz, 1);
    transform.translation = float4(DTid.x, DTid.x % 10, 0, 1);

    transformBuffer_1[DTid.x] = transform;
}