struct VERTEX_IN
{
	float3 position : POSITION;
	float4 color : COLOR;
};

struct VERTEX_OUT
{
	float4 position : SV_Position;
	float4 color : COLOR;
};

cbuffer Transform
{
	float4x4 worldTransform;
	float4x4 viewTransform;
	float4x4 projectionTransform;
};

VERTEX_OUT main (VERTEX_IN vin)
{
	VERTEX_OUT vout;
	vout.position = mul (float4 (vin.position, 1), worldTransform);
	vout.position = mul (vout.position, viewTransform);
	vout.position = mul (vout.position, projectionTransform);
	vout.color = vin.color;

	return vout;
}