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
	float sum = color.x + color.y + color.z;
	float4 filter = input.normal;
	if (sum > 1.5f)	 // set 1.5 to 0 to also filter spiderman
	{
		if (filter.x < 0.0f || filter.y < 0.0f || filter.z < 0.0f)
			filter += float4(1.0f, 1.0f , 1.0f, 1.0f);
		float minlim = 0.3f;
		filter.x = max(minlim, filter.x);
		filter.y = max(minlim, filter.y);
		filter.z = max(minlim, filter.z);
		color *= filter;
	}
	return color;
}