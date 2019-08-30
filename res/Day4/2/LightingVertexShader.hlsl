struct VERTEX_IN
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texcoord : TEXCOORD;
	float4 color : COLOR;
};

struct VERTEX_OUT
{
	float4 position : SV_Position;
	float4 color : COLOR;
};

cbuffer Transform : register (b0)
{
	float4x4 worldTransform;
	float4x4 viewTransform;
	float4x4 projectionTransform;
};

cbuffer LightSource : register (b1)
{
	float4 pointLights[256];
};

VERTEX_OUT main (VERTEX_IN vin)
{
	VERTEX_OUT vout;
	vout.position = mul (float4 (vin.position, 1), worldTransform);

	float3 worldNormal = normalize (mul (vin.normal, (float3x3)worldTransform));

	float4 color = float4 (0, 0, 0, 1);
	for (int i = 0; i < 256; ++i)
	{
		if (pointLights[i].w)
		{
			float3 lightDir = vout.position.xyz - pointLights[i].xyz;
			float3 nLightDir = normalize (lightDir);
			float3 calced = saturate ((float3) dot (-nLightDir, worldNormal));
			color += float4 (calced, 1);
		}
	}

	vout.position = mul (vout.position, viewTransform);
	vout.position = mul (vout.position, projectionTransform);

	vout.color = color * vin.color;

	return vout;
}