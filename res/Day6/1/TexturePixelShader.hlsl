struct PIXEL_IN
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD;
};

SamplerState samplerState;
Texture2D tex;

float4 main(PIXEL_IN pin) : SV_Target
{
    return tex.Sample(samplerState, pin.texcoord);
}