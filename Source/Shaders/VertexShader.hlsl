struct VS_in {
	float3 pos : POSITION;
	float2 uv: UV;
};

struct VS_out {
	//float4 posW : POSITIONH; //Not needed right not, but might be in the future
	float4 posH : SV_POSITION;
	float2 uv : UV;
};

cbuffer matrixBuffer : register(b0)
{
	float4x4 viewMatrix;
	float4x4 projMatrix;
	float4x4 padd1[2];
}

cbuffer transformBuffer : register(b1)
{
	float4x4 transform;
	float4 padd2[12];
}

VS_out main( VS_in input )
{
	VS_out output;

	output.posH = mul(transform, float4(input.pos, 1.0f));
	output.posH = mul(viewMatrix, output.posH);
	output.posH = mul(projMatrix, output.posH);
	output.uv = input.uv;

	return output;
}