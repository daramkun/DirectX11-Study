float4 main (float3 position : POSITION) : SV_Position
{
	return float4 (position, 1);
}