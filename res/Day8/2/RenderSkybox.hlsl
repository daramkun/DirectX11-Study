struct VERTEX_IN
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 texcoord : TEXCOORD;
    float4 color : COLOR;
};

struct PIXEL_IN
{
    float4 position : SV_Position;
    float3 texcoord : TEXCOORD;
};

cbuffer TRANSFORM
{
    float4x4 world : WORLD;
    float4x4 view : VIEW;
    float4x4 proj : PROJECTION;
};

PIXEL_IN vs_main(VERTEX_IN vin)
{
    PIXEL_IN pin;
    pin.position = float4(vin.position, 1);
    pin.position = mul(pin.position, world);
    pin.position = mul(pin.position, view);
    pin.position = mul(pin.position, proj);
    pin.texcoord = normalize(vin.position); //vin.texcoord;

    return pin;
}

SamplerState samplerState;
TextureCube tex;

float4 ps_main(PIXEL_IN pin) : SV_Target
{
    return tex.Sample(samplerState, pin.texcoord);
}