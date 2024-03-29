struct VS_in {
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 uv: UV;
	float3 instancePos : INSTANCEPOS;
	uint instanceID : SV_InstanceID;
};

struct VS_out {
	//float4 posW : POSITIONH; //Not needed right not, but might be in the future
	float4 posH : SV_POSITION;
	float4 normal : NORMAL;
	float2 uv : UV;
};

cbuffer matrixBuffer : register(b0)
{
	float4x4 viewMatrix;
	float4x4 projMatrix;
	float dt;
	float3 smallPadd;
	float4 matrixPadd[7];
}

RWStructuredBuffer <float4x4> transformBuffer : register(u0);

VS_out main( VS_in input )
{
	VS_out output;

	float4 pos = float4(input.pos, 1.0f);



	output.posH = mul(transformBuffer[input.instanceID], pos);
	output.posH += float4(input.instancePos, 1.0f);
	output.posH = mul(viewMatrix, output.posH);
	output.posH = mul(projMatrix, output.posH);
	output.normal = float4(input.normal, 1.0f);
	output.uv = input.uv;

	return output;
}