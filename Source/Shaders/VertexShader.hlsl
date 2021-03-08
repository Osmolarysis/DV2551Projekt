struct VS_in {
	float4 pos : POSITION;
	float4 colour : COLOR;
};

struct VS_out {
	//float4 posW : POSITIONH; //Not needed right not, but might be in the future
	float4 posH : SV_POSITION;
	float4 colour : COLOR;
};

VS_out main( VS_in input )
{
	VS_out output;

	output.posH = mul(input.pos, float4(1, 1, 1, 1)/*worldViewPerspective matrix from camera*/);
	output.colour = input.colour;

	return output;
}