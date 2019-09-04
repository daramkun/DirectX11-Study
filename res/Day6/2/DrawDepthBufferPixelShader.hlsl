struct PIXEL_IN
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD;
};

SamplerState samplerState
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
};

Texture2D tex;

float4 main(PIXEL_IN pin) : SV_Target
{
    float4 depth = tex.Sample(samplerState, pin.texcoord);
    float depthValue = 1 - depth.r;
    return float4(depthValue, depthValue, depthValue, 1);
}