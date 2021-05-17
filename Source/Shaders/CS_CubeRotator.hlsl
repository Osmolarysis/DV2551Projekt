cbuffer matrixBuffer : register(b0)
{
	float4x4 viewMatrix;
	float4x4 projMatrix;
	float dt;
	float3 smallPadd;
	float4 matrixPadd[7];
}

RWStructuredBuffer <float4x4> transformUpdateBuffer : register(u0);
RWStructuredBuffer <float4x4> transformReadBuffer : register(u1);

static const float PI = 3.14159265358979f;

[numthreads(1024, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	float rotValue = dt;

	//Read data
	float4x4 transform = transformUpdateBuffer[DTid.x];

	// Update
	float4x4 rotationX = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, cos(rotValue), -sin(rotValue), 0.0f,
		0.0f, sin(rotValue), cos(rotValue), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	float4x4 rotationY = {
		cos(rotValue), 0.0f, sin(rotValue), 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		-sin(rotValue), 0, cos(rotValue), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	float4x4 rotationZ = {
		cos(rotValue), -sin(rotValue), 0.0f, 0.0f,
		sin(rotValue), cos(rotValue), 0.0, 0.0f,
		0.0f, 0.0f, 1, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	transform = mul(transform, rotationX);
	transform = mul(transform, rotationY);
	transform = mul(transform, rotationZ);


	//Write data
	transformUpdateBuffer[DTid.x] = transform;
	transformReadBuffer[DTid.x] = transform;
}