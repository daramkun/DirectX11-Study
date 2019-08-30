struct VERTEX_IN
{
	float3 position : POSITION;
	float2 texcoord : TEXCOORD;
};

struct VERTEX_OUT
{
	float4 position : SV_Position;
	float2 texcoord : TEXCOORD;
};

VERTEX_OUT main (VERTEX_IN vin)
{
	VERTEX_OUT vout;
	vout.position = float4 (vin.position, 1);
	vout.texcoord = vin.texcoord;

	return vout;
}