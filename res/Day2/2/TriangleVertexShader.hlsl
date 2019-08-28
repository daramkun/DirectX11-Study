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
};

VERTEX_OUT main (VERTEX_IN vin)
{
	VERTEX_OUT vout;
	vout.position = mul (worldTransform, float4 (vin.position, 1));
	vout.color = vin.color;

	return vout;
}