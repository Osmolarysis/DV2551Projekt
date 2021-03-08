struct PS_in {
	float4 PosH : SV_POSITION;
	float4 colour : COLOR;
};

float4 main(PS_in input) : SV_TARGET
{

	return float4(input.colour);
}