struct VS_in {
	float4 pos : POSITION;
	float4 colour : COLOR;
};

struct VS_out {
	//float4 posW : POSITIONH; //Not needed right not, but might be in the future
	float4 posH : SV_POSITION;
	float4 colour : COLOR;
};

cbuffer matrixBuffer : register(b0)
{
	float4x4 viewMatrix;
	float4x4 projMatrix;
	float4x4 padd1[2];
}

cbuffer transformBuffer : register(b1)
{
	float4 translate;
	float4 padd2[15];
}

VS_out main( VS_in input )
{
	VS_out output;

	output.posH = input.pos + translate;
	output.posH = mul(output.posH, viewMatrix);
	output.posH = mul(output.posH, projMatrix);
	output.colour = input.colour;

	return output;
}