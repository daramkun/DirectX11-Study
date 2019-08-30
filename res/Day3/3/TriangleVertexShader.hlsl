struct VERTEX_OUT
{
	float4 position : SV_Position;
	float4 color : COLOR;
};

VERTEX_OUT main (uint vertexId : SV_VertexID)
{
	VERTEX_OUT vout;

	switch (vertexId)
	{
	case 0:
		vout.position = float4 (-0.5f, -0.5f, 0, 1);
		vout.color = float4 (1, 0, 0, 1);
		break;

	case 1:
		vout.position = float4 (+0.0f, +0.5f, 0, 1);
		vout.color = float4 (0, 1, 0, 1);
		break;

	case 2:
		vout.position = float4 (+0.5f, -0.5f, 0, 1);
		vout.color = float4 (0, 0, 1, 1);
		break;
	}

	return vout;
}