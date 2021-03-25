struct PS_in {
	float4 PosH : SV_POSITION;
	float4 normal : NORMAL;
	float2 uv : UV;
};

Texture2D diffuseMap : register(t0);
SamplerState mySampler : register(s0);

float4 main(PS_in input) : SV_TARGET
{
	float4 color = diffuseMap.Sample(mySampler, input.uv);
	float4 filter = input.normal;
	if (filter.x < 0.0f || filter.y < 0.0f || filter.z < 0.0f)
		filter += float4(1.0f, 1,1,0);
	color *= filter;
	return color;
}