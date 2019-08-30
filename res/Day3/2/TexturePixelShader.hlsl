struct PIXEL_IN
{
	float4 position : SV_Position;
	float2 texcoord : TEXCOORD;
};

SamplerState textureSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};
Texture2D tex1, tex2;

float4 main (PIXEL_IN pin) : SV_Target
{
	return (tex1.Sample (textureSampler, pin.texcoord)
	+ tex2.Sample(textureSampler, pin.texcoord)) / 2;
}