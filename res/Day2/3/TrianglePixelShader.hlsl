struct PIXEL_IN
{
	float4 position : SV_Position;
	float4 color : COLOR;
};

float4 main (PIXEL_IN pin) : SV_Target
{
	return pin.color;
}