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

#define NEAR												0.0001f
#define FAR													1000.0f

cbuffer TRANSFORM
{
    float4x4 world;
    float4x4 view;
    float4x4 proj;
};

float4 main(PIXEL_IN pin) : SV_Target
{
    float4 depth = tex.Sample(samplerState, pin.texcoord);
    float depthValue = depth.x * 2.0f - 1.0f;
    depthValue = (2.0f * NEAR) / (FAR + NEAR - depth.x * (FAR - NEAR));
    return saturate(float4(depthValue, depthValue, depthValue, 1));
}