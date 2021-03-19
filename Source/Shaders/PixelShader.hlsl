struct PS_in {
	float4 PosH : SV_POSITION;
	float2 uv : UV;
};

Texture2D diffuseMap : register(t0);
SamplerState mySampler : register(s0);

float4 main(PS_in input) : SV_TARGET
{
	float4 color = diffuseMap.Sample(mySampler, input.uv);
	return color;
}